#include "DoubleRatchet.h"
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace Crypto;

// Helper to print vectors
static void printHex(const std::string &label, const std::vector<uint8_t> &data) {
  std::cerr << label << " [" << data.size() << "] = ";
  for (const auto b: data) std::cerr << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
  std::cerr << std::dec << std::endl;
}

DoubleRatchet::DoubleRatchet(SessionType sessionType, ConstructType constructType, RatchetState *ratchetState,
                             std::unique_ptr<KeyEnv> keyEnv, const std::vector<uint8_t> &theirPub)
  : state(std::make_unique<RatchetState>()),
    ownKeyEnv(std::make_unique<KeyEnv>(KeyType::X25519Keypair)),
    theirKeyEnv(std::make_unique<KeyEnv>(KeyType::X25519Keypair)),
    encryptionEnv(std::make_unique<EncryptionEnv>(EncAlgorithm::AES256)),
    decryptionEnv(std::make_unique<EncryptionEnv>(EncAlgorithm::AES256)),
    kdfEnv(std::make_unique<KDFEnv>(KDFType::SHA3_512)),
    hashingEnv(std::make_unique<HashingEnv>(HashAlgorithm::BLAKE2b512)) {
  // Initialisiere den Zufallsgenerator einmal
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  state->sessionType = sessionType;
  std::cerr << "[ctor] SessionType=" << static_cast<int>(sessionType) << " ConstructType=" << static_cast<int>(
    constructType) << std::endl;
  if (constructType == ConstructType::INIT) {
    generateKeypair();
    initNewSession(theirPub);
  } else if (constructType == ConstructType::FOLLOWINIT) {
    if (keyEnv != nullptr) {
      importKeyEnv(std::move(keyEnv));
    }
    initNewSession(theirPub);
  } else if (constructType == ConstructType::EXISTING && ratchetState) {
    setState(ratchetState);
  }
}

bool DoubleRatchet::setState(RatchetState *rs) {
  if (rs) {
    *state = *rs;
    return true;
  }
  return false;
}

bool DoubleRatchet::importKeyEnv(std::unique_ptr<KeyEnv> keyEnv) {
  if (!keyEnv) return false;
  // Create a new KeyEnv instance instead of taking ownership
  ownKeyEnv = std::move(keyEnv);
  state->ownPrivKey = ownKeyEnv->getPrivateRaw();
  state->ownPubKey = ownKeyEnv->getPublicRaw();
  return true;
}

bool DoubleRatchet::generateKeypair() {
  ownKeyEnv->startKeyPairGeneration(true);
  state->ownPrivKey = ownKeyEnv->getPrivateRaw();
  state->ownPubKey = ownKeyEnv->getPublicRaw();
  printHex("[generateKeypair] PrivKey", state->ownPrivKey);
  printHex("[generateKeypair] PubKey", state->ownPubKey);
  return true;
}

bool DoubleRatchet::deriveSharedSecret(const std::vector<uint8_t> &theirPub) {
  if (theirPub.size() != 32) {
    std::cerr << "[deriveSharedSecret] Invalid theirPub size=" << theirPub.size() << std::endl;
    return false;
  }
  state->theirPubKey = theirPub;
  // Set the private key before deriving the shared secret
  state->sharedSecret = ownKeyEnv->deriveSharedSecret(theirPub);
  printHex("[deriveSharedSecret] theirPub", theirPub);
  printHex("[deriveSharedSecret] sharedSecret", state->sharedSecret);
  return true;
}

bool DoubleRatchet::initRootChain() {
  if (state->sharedSecret.empty()) {
    std::cerr << "[initRootChain] Error: Empty shared secret" << std::endl;
    return false;
  }

  std::vector<uint8_t> salt(16);
  for (size_t i = 0; i < 16; ++i) salt[i] = uint8_t(i);

  state->rootKey.resize(32);
  kdfEnv->startKDF(state->sharedSecret, salt, "InitialRootKey", state->rootKey, 32);
  state->sendChainKey = state->rootKey;
  state->recvChainKey = state->rootKey; // Initialize receive chain key as well

  printHex("[initRootChain] rootKey", state->rootKey);
  printHex("[initRootChain] sendChainKey", state->sendChainKey);
  printHex("[initRootChain] recvChainKey", state->recvChainKey);
  return true;
}

bool DoubleRatchet::initNewSession(const std::vector<uint8_t> &theirPub) {
  std::cerr << "[initNewSession] starting" << std::endl;
  deriveSharedSecret(theirPub);
  return initRootChain();
}

bool DoubleRatchet::symmetricRatchetStep() {
  const size_t OUTLEN = 64;
  std::vector<uint8_t> out(OUTLEN);

  // Check if sendChainKey is initialized
  if (state->sendChainKey.empty()) {
    std::cerr << "[symmetricRatchetStep] Error: sendChainKey is empty" << std::endl;
    return false;
  }

  // Check if sharedSecret is initialized
  if (state->sharedSecret.empty()) {
    std::cerr << "[symmetricRatchetStep] Error: sharedSecret is empty" << std::endl;
    return false;
  }

  if (!kdfEnv->startKDF(state->sendChainKey, state->sharedSecret, "SendChainStep", out, OUTLEN)) {
    std::cerr << "[symmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }

  std::vector<uint8_t> newChain(out.begin(), out.begin() + 32);
  std::vector<uint8_t> msgKey(out.begin() + 32, out.end());
  printHex("[symmetricRatchetStep] out", out);
  printHex("[symmetricRatchetStep] newChain", newChain);
  printHex("[symmetricRatchetStep] msgKey", msgKey);

  // Store the new chain key and message key
  state->sendChainKey = std::move(newChain);
  state->message_keys[state->send_msg_num] = std::move(msgKey);
  std::cerr << "[symmetricRatchetStep] msg_num=" << state->send_msg_num << std::endl;
  state->send_msg_num++;
  return true;
}

bool DoubleRatchet::receiveSymmetricRatchetStep(uint32_t msg_num) {
  const size_t OUTLEN = 64;
  std::vector<uint8_t> out(OUTLEN);

  // Use sendChainKey instead of recvChainKey for symmetric operation
  if (state->sendChainKey.empty()) {
    std::cerr << "[receiveSymmetricRatchetStep] Error: sendChainKey is empty" << std::endl;
    return false;
  }

  if (state->sharedSecret.empty()) {
    std::cerr << "[receiveSymmetricRatchetStep] Error: sharedSecret is empty" << std::endl;
    return false;
  }

  // Use same KDF parameters as in symmetricRatchetStep
  if (!kdfEnv->startKDF(state->sendChainKey, state->sharedSecret, "SendChainStep", out, OUTLEN)) {
    std::cerr << "[receiveSymmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }

  std::vector<uint8_t> newChain(out.begin(), out.begin() + 32);
  std::vector<uint8_t> msgKey(out.begin() + 32, out.end());

  // Store the new chain key and message key
  state->sendChainKey = std::move(newChain);
  state->message_keys[msg_num] = std::move(msgKey);

  return true;
}

bool DoubleRatchet::encryptMessage(const std::vector<uint8_t> &msg,
                                   std::vector<uint8_t> &cipher,
                                   std::vector<uint8_t> &tag,
                                   std::vector<uint8_t> &iv) {
  std::cerr << "[encryptMessage] plaintext size=" << msg.size() << std::endl;
  printHex("[encryptMessage] plaintext", msg);

  // Perform symmetric ratchet step to generate the message key
  if (!symmetricRatchetStep()) return false;

  // Get the message key that was just created
  // Check if the key exists before accessing it
  if (state->message_keys.find(state->send_msg_num - 1) == state->message_keys.end()) {
    std::cerr << "[encryptMessage] Error: Missing message key for send_msg_num=" << (state->send_msg_num - 1) <<
        std::endl;
    return false;
  }

  auto &key = state->message_keys[state->send_msg_num - 1];
  printHex("[encryptMessage] msgKey", key);

  // Set up encryption environment
  encryptionEnv->key = key;
  encryptionEnv->iv = iv;
  encryptionEnv->plaintext = msg;
  encryptionEnv->ciphertext = cipher;
  encryptionEnv->authTag = tag;

  if (!encryptionEnv->startEncryption()) {
    std::cerr << "[encryptMessage] Encryption failed" << std::endl;
    return false;
  } else {
    std::cerr << "[encryptMessage] Encryption succeeded" << std::endl;
    cipher = encryptionEnv->ciphertext;
    tag = encryptionEnv->authTag;
  }

  printHex("[encryptMessage] iv", encryptionEnv->iv);
  printHex("[encryptMessage] cipher", cipher);
  printHex("[encryptMessage] authTag", tag);
  return true;
}

bool DoubleRatchet::decryptMessage(const std::vector<uint8_t> &cipher,
                                   std::vector<uint8_t> &msg,
                                   const std::vector<uint8_t> &tag,
                                   const std::vector<uint8_t> &iv,
                                   uint32_t num) {
  std::cerr << "[decryptMessage] cipher size=" << cipher.size() << " iv size=" << iv.size() << " tag size=" << tag.
      size() << " msgNum=" << num << std::endl;
  printHex("[decryptMessage] cipher", cipher);
  printHex("[decryptMessage] authTag", tag);
  printHex("[decryptMessage] iv", iv);

  // Generate message key if not already present
  if (state->message_keys.find(num) == state->message_keys.end()) {
    std::cerr << "[decryptMessage] generating msgKey for num=" << num << std::endl;
    if (!receiveSymmetricRatchetStep(num)) {
      std::cerr << "[decryptMessage] Failed to generate message key" << std::endl;
      return false;
    }
  }

  auto &key = state->message_keys[num];
  printHex("[decryptMessage] msgKey", key);

  // Set up decryption environment
  decryptionEnv->key = key;
  decryptionEnv->iv = iv;
  decryptionEnv->ciphertext = cipher;
  decryptionEnv->authTag = tag;
  decryptionEnv->plaintext = msg;

  if (!decryptionEnv->startDecryption()) {
    std::cerr << "[decryptMessage] Decryption failed" << std::endl;
    return false;
  } else {
    std::cerr << "[decryptMessage] Decryption succeeded" << std::endl;
    msg = decryptionEnv->plaintext;
  }

  printHex("[decryptMessage] plaintext", msg);
  return true;
}

std::string DoubleRatchet::packEncMessage(const std::string &plaintext) {
  std::vector<uint8_t> msg(plaintext.begin(), plaintext.end()), cipher, tag;
  std::cerr << "[packEncMessage] plaintext '" << plaintext << "'" << std::endl;

  KeyEnv keyEnv(KeyType::KeyIv);
  keyEnv.setKeyIvSizes(32, 12);
  std::vector<uint8_t> iv;
  std::vector<uint8_t> key;

  if (!keyEnv.startKeyIvGeneration(key, iv)) {
    return "";
  }

  printHex("[packEncMessage] generated iv", iv);

  if (!encryptMessage(msg, cipher, tag, iv)) {
    std::cerr << "[packEncMessage] encryption failed" << std::endl;
    throw std::runtime_error("DoubleRatchet::packEncMessage: encryption failed");
  }

  // Ensure the current message number is used
  uint32_t currentMsgNum = state->send_msg_num - 1;

  std::cout << "iv size=" << iv.size() << std::endl;
  std::cout << "tag size=" << tag.size() << std::endl;
  std::cout << "ownPubKey size=" << state->ownPubKey.size() << std::endl;
  std::cout << "theirPubKey size=" << state->theirPubKey.size() << std::endl;

  if (iv.size() != 12 || tag.size() != 16 || state->ownPubKey.size() != 32 || state->theirPubKey.size() != 32) {
    std::cerr << "[packEncMessage] Error: Invalid header element sizes" << std::endl;
    return "";
  }

  RatchetHeader hdr{
    iv, tag, state->ownPubKey, state->theirPubKey, currentMsgNum,
    static_cast<uint32_t>(msg.size())
  };

  printHex("[packEncMessage] hdr.iv", hdr.iv);
  printHex("[packEncMessage] hdr.authTag", hdr.authTag);
  printHex("[packEncMessage] hdr.SenderPubKey", hdr.SenderPubKey);
  printHex("[packEncMessage] hdr.ReceiverPubKey", hdr.ReceiverPubKey);
  std::cerr << "[packEncMessage] hdr.sendMessageNum=" << hdr.sendMessageNum << std::endl;
  std::cerr << "[packEncMessage] hdr.messageLength=" << hdr.messageLength << std::endl;

  std::string out;
  out.insert(out.end(), hdr.iv.begin(), hdr.iv.end());
  out.insert(out.end(), hdr.authTag.begin(), hdr.authTag.end());
  out.insert(out.end(), hdr.SenderPubKey.begin(), hdr.SenderPubKey.end());
  out.insert(out.end(), hdr.ReceiverPubKey.begin(), hdr.ReceiverPubKey.end());
  out.append(reinterpret_cast<const char *>(&hdr.sendMessageNum), sizeof(uint32_t));
  out.append(reinterpret_cast<const char *>(&hdr.messageLength), sizeof(uint32_t));
  out.insert(out.end(), cipher.begin(), cipher.end());
  return out;
}

std::string DoubleRatchet::unpackDecMessage(const std::string &pkg) {
  if (pkg.empty()) {
    std::cerr << "[unpackDecMessage] Error: Empty package" << std::endl;
    return "";
  }

  if (pkg.length() < 100) {
    // 12 (iv) + 16 (tag) + 32 (spk) + 32 (rpk) + 4 (num) + 4 (len)
    std::cerr << "[unpackDecMessage] Error: Package too short, size=" << pkg.length() << std::endl;
    return "";
  }

  size_t pos = 0;
  auto read = [&](size_t n) {
    if (pos + n > pkg.length()) {
      std::cerr << "[unpackDecMessage] Error: Trying to read beyond package bounds" << std::endl;
      return std::vector<uint8_t>();
    }
    auto v = std::vector<uint8_t>(pkg.begin() + pos, pkg.begin() + pos + n);
    pos += n;
    return v;
  };

  auto iv = read(12);
  if (iv.empty()) return "";

  auto tag = read(16);
  if (tag.empty()) return "";

  auto spk = read(32);
  if (spk.empty()) return "";

  auto rpk = read(32);
  if (rpk.empty()) return "";

  if (pos + 8 > pkg.length()) {
    std::cerr << "[unpackDecMessage] Error: Package too short for message number and length" << std::endl;
    return "";
  }

  uint32_t num, len;
  std::memcpy(&num, pkg.data() + pos, 4);
  pos += 4;
  std::memcpy(&len, pkg.data() + pos, 4);
  pos += 4;

  if (pos > pkg.length()) {
    std::cerr << "[unpackDecMessage] Error: Package too short for cipher data" << std::endl;
    return "";
  }

  std::vector<uint8_t> cipher(pkg.begin() + pos, pkg.end());
  std::cerr << "[unpackDecMessage] num=" << num << " len=" << len << std::endl;
  printHex("[unpackDecMessage] iv", iv);
  printHex("[unpackDecMessage] tag", tag);
  printHex("[unpackDecMessage] spk", spk);

  // Check if we need to do an asymmetric ratchet step
  if (spk != state->theirPubKey) {
    std::cerr << "[unpackDecMessage] Performing asymmetric ratchet step" << std::endl;
    asymmetricRatchetStep(spk);
  }

  std::vector<uint8_t> out;
  if (!decryptMessage(cipher, out, tag, iv, num)) {
    std::cerr << "[unpackDecMessage] Failed to decrypt message" << std::endl;
    return "";
  }

  return std::string(out.begin(), out.end());
}

bool DoubleRatchet::updateRootKey(const std::vector<uint8_t> &newPubKey) {
  std::cerr << "[updateRootKey] newPubKey" << std::endl;
  printHex("[updateRootKey] newPubKey", newPubKey);

  // Ensure the new public key is set before the asymmetric ratchet step
  state->theirPubKey = newPubKey;

  return asymmetricRatchetStep(newPubKey);
}

RatchetState *DoubleRatchet::getState() const {
  std::cerr << "[getState] send_msg_num=" << state->send_msg_num << " recv_msg_num=" << state->recv_msg_num <<
      std::endl;
  return state.get();
}

std::vector<uint8_t> DoubleRatchet::ownPubKey() const {
  printHex("[ownPubKey] ownPubKey", state->ownPubKey);
  return state->ownPubKey;
}

bool DoubleRatchet::asymmetricRatchetStep(const std::vector<uint8_t> &theirPub) {
  std::cerr << "[asymmetricRatchetStep] starting" << std::endl;

  if (theirPub.empty()) {
    std::cerr << "[asymmetricRatchetStep] Error: Empty theirPub" << std::endl;
    return false;
  }

  // Save current own key pair before generating a new one
  std::vector<uint8_t> oldPrivKey = state->ownPrivKey;
  std::vector<uint8_t> oldPubKey = state->ownPubKey;

  // Generate a new key pair
  generateKeypair();

  // Set the theirPubKey
  state->theirPubKey = theirPub;

  // Derive the shared secret using our new private key and their public key
  state->sharedSecret = ownKeyEnv->deriveSharedSecret(theirPub);
  printHex("[asymmetricRatchetStep] sharedSecret", state->sharedSecret);

  // Derive new root key
  std::vector<uint8_t> newRoot(32);
  if (!kdfEnv->startKDF(state->rootKey, state->sharedSecret, "DH-Ratchet-Update", newRoot, 32)) {
    std::cerr << "[asymmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }
  printHex("[asymmetricRatchetStep] newRoot", newRoot);

  // Update state
  state->rootKey = std::move(newRoot);
  state->sendChainKey = state->rootKey;
  state->recvChainKey = state->rootKey;

  // Reset message counters for new chain
  state->send_msg_num = 0;
  state->recv_msg_num = 0;

  printHex("[asymmetricRatchetStep] sendChainKey", state->sendChainKey);
  printHex("[asymmetricRatchetStep] recvChainKey", state->recvChainKey);
  return true;
}

bool DoubleRatchet::testOneSideDoubleRatchet() {
  std::cerr << "[testDoubleRatchet] start" << std::endl;

  std::unique_ptr<KeyEnv> bobKeyEnv = std::make_unique<KeyEnv>(KeyType::X25519Keypair);
  bobKeyEnv->startKeyPairGeneration(true);
  auto bobPubKey = bobKeyEnv->getPublicRaw();

  std::cout << "[testDoubleRatchet] created Bob's KeyEnv and got public key" << std::endl;
  printHex("[testDoubleRatchet] bobPubKey", bobPubKey);

  DoubleRatchet alice(SessionType::DUO, ConstructType::INIT, nullptr, nullptr, bobPubKey);
  std::cout << "[testDoubleRatchet] created test alice" << std::endl;

  // Get Alice's public key
  auto alicePubKey = alice.ownPubKey();
  printHex("[testDoubleRatchet] alicePubKey", alicePubKey);

  // Create Bob with Alice's pubkey
  DoubleRatchet bob(SessionType::DUO, ConstructType::FOLLOWINIT, nullptr, std::move(bobKeyEnv), alicePubKey);
  std::cout << "[testDoubleRatchet] created test bob" << std::endl;

  // Now the shared secrets should match
  if (alice.getState()->sharedSecret == bob.getState()->sharedSecret) {
    std::cerr << "[testDoubleRatchet] shared secrets match" << std::endl;
  } else {
    std::cerr << "[testDoubleRatchet] shared secrets do not match" << std::endl;
    printHex("[testDoubleRatchet] alice sharedSecret", alice.getState()->sharedSecret);
    printHex("[testDoubleRatchet] bob sharedSecret", bob.getState()->sharedSecret);
    return false;
  }

  // Test encryption/decryption
  bool ok = true;
  for (size_t i = 0; i < 1; ++i) {
    std::string msg = "Test Nachricht: " + std::to_string(i);
    std::cerr << "[testDoubleRatchet] encrypting message: " << msg << std::endl;

    try {
      auto pkg = alice.packEncMessage(msg);
      if (pkg == "") {
        std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
        ok = false;
        continue;
      }
      std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

      auto dec = bob.unpackDecMessage(pkg);
      std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;

      if (dec != msg) {
        std::cerr << "[testDoubleRatchet] Error: decrypted message does not match original" << std::endl;
        ok = false;
      } else {
        std::cerr << "[testDoubleRatchet] Success: message correctly encrypted and decrypted" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "[testDoubleRatchet] Exception: " << e.what() << std::endl;
      ok = false;
    }
  }

  return ok;
}

bool DoubleRatchet::testMixedDoubleRatchet() {
  std::cerr << "[testDoubleRatchet] start" << std::endl;

  std::unique_ptr<KeyEnv> bobKeyEnv = std::make_unique<KeyEnv>(KeyType::X25519Keypair);
  bobKeyEnv->startKeyPairGeneration(true);
  auto bobPubKey = bobKeyEnv->getPublicRaw();

  std::cout << "[testDoubleRatchet] created Bob's KeyEnv and got public key" << std::endl;
  printHex("[testDoubleRatchet] bobPubKey", bobPubKey);

  DoubleRatchet alice(SessionType::DUO, ConstructType::INIT, nullptr, nullptr, bobPubKey);
  std::cout << "[testDoubleRatchet] created test alice" << std::endl;

  // Get Alice's public key
  auto alicePubKey = alice.ownPubKey();
  printHex("[testDoubleRatchet] alicePubKey", alicePubKey);

  // Create Bob with Alice's pubkey
  DoubleRatchet bob(SessionType::DUO, ConstructType::FOLLOWINIT, nullptr, std::move(bobKeyEnv), alicePubKey);
  std::cout << "[testDoubleRatchet] created test bob" << std::endl;

  // Now the shared secrets should match
  if (alice.getState()->sharedSecret == bob.getState()->sharedSecret) {
    std::cerr << "[testDoubleRatchet] shared secrets match" << std::endl;
  } else {
    std::cerr << "[testDoubleRatchet] shared secrets do not match" << std::endl;
    printHex("[testDoubleRatchet] alice sharedSecret", alice.getState()->sharedSecret);
    printHex("[testDoubleRatchet] bob sharedSecret", bob.getState()->sharedSecret);
    return false;
  }

  // Test encryption/decryption
  bool ok = true;
  for (size_t i = 0; i < 2; ++i) {
    std::string msg = "Test Nachricht: " + std::to_string(i);
    std::cerr << "[testDoubleRatchet] encrypting message: " << msg << std::endl;

    try {
      std::string dec;
      if (i % 2 == 0) {
        auto pkg = alice.packEncMessage(msg);
        if (pkg == "") {
          std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
          ok = false;
          continue;
        }
        std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

        dec = bob.unpackDecMessage(pkg);
        std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;
      } else {
        auto pkg = bob.packEncMessage(msg);
        if (pkg == "") {
          std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
          ok = false;
          continue;
        }
        std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

        dec = alice.unpackDecMessage(pkg);
        std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;
      }


      if (dec != msg) {
        std::cerr << "[testDoubleRatchet] Error: decrypted message does not match original" << std::endl;
        ok = false;
      } else {
        std::cerr << "[testDoubleRatchet] Success: message correctly encrypted and decrypted" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "[testDoubleRatchet] Exception: " << e.what() << std::endl;
      ok = false;
    }
  }

  return ok;
}
