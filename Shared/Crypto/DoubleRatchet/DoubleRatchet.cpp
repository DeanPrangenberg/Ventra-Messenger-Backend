#include "DoubleRatchet.h"

#define printHexDebug false
#define printNormDebug true
#define testAmount 1

using namespace Crypto;

DoubleRatchet::DoubleRatchet(SessionType sessionType, ConstructType constructType, RatchetState *ratchetState,
                             std::unique_ptr<KeyEnv> keyEnv, const std::vector<uint8_t> &theirPub)
    : state(std::make_unique<RatchetState>()),
      ownKeyEnv(std::make_unique<KeyEnv>(KeyType::X25519Keypair)),
      theirKeyEnv(std::make_unique<KeyEnv>(KeyType::X25519Keypair)),
      encryptionEnv(std::make_unique<EncryptionEnv>(EncAlgorithm::AES256)),
      decryptionEnv(std::make_unique<EncryptionEnv>(EncAlgorithm::AES256)),
      kdfEnv(std::make_unique<KDFEnv>(KDFType::SHA3_512)),
      hashingEnv(std::make_unique<HashingEnv>(HashAlgorithm::BLAKE2b512)) {

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    state->sessionType = sessionType;

    // Add special handling for multi-chat sessions
    if (sessionType == SessionType::MULTI) {
        // Use a different KDF configuration for multi-chat
        kdfEnv = std::make_unique<KDFEnv>(KDFType::SHA3_512);
    }

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
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[generateKeypair] PrivKey", state->ownPrivKey);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[generateKeypair] PubKey", state->ownPubKey);
  return true;
}

bool DoubleRatchet::deriveSharedSecret(const std::vector<uint8_t> &theirPub) {
  if (theirPub.size() != 32) {
    if constexpr (printNormDebug) std::cerr << "[deriveSharedSecret] Invalid theirPub size=" << theirPub.size() << std::endl;
    return false;
  }
  state->theirPubKey = theirPub;
  // Set the private key before deriving the shared secret
  state->sharedSecret = ownKeyEnv->deriveSharedSecret(theirPub);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[deriveSharedSecret] theirPub", theirPub);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[deriveSharedSecret] sharedSecret", state->sharedSecret);
  return true;
}

bool DoubleRatchet::initRootChain() {
  if (state->sharedSecret.empty()) {
    if constexpr (printNormDebug) std::cerr << "[initRootChain] Error: Empty shared secret" << std::endl;
    return false;
  }

  std::vector<uint8_t> salt(16);
  for (size_t i = 0; i < 16; ++i) salt[i] = uint8_t(i);

  state->rootKey.resize(32);
  kdfEnv->startKDF(state->sharedSecret, salt, "InitialRootKey", state->rootKey, 32);
  state->sendChainKey = state->rootKey;
  state->recvChainKey = state->rootKey; // Initialize receive chain key as well

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[initRootChain] rootKey", state->rootKey);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[initRootChain] sendChainKey", state->sendChainKey);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[initRootChain] recvChainKey", state->recvChainKey);
  return true;
}

bool DoubleRatchet::initNewSession(const std::vector<uint8_t> &theirPub) {
  if constexpr (printNormDebug) std::cerr << "[initNewSession] starting" << std::endl;
  deriveSharedSecret(theirPub);
  return initRootChain();
}

bool DoubleRatchet::symmetricRatchetStep() {
  const size_t OUTLEN = 64;
  std::vector<uint8_t> out(OUTLEN);

  // Check if sendChainKey is initialized
  if (state->sendChainKey.empty()) {
    if constexpr (printNormDebug) std::cerr << "[symmetricRatchetStep] Error: sendChainKey is empty" << std::endl;
    return false;
  }

  // Check if sharedSecret is initialized
  if (state->sharedSecret.empty()) {
    if constexpr (printNormDebug) std::cerr << "[symmetricRatchetStep] Error: sharedSecret is empty" << std::endl;
    return false;
  }

  if (!kdfEnv->startKDF(state->sendChainKey, state->sharedSecret, "SendChainStep", out, OUTLEN)) {
    if constexpr (printNormDebug) std::cerr << "[symmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }

  std::vector<uint8_t> newChain(out.begin(), out.begin() + 32);
  std::vector<uint8_t> msgKey(out.begin() + 32, out.end());
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[symmetricRatchetStep] out", out);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[symmetricRatchetStep] newChain", newChain);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[symmetricRatchetStep] msgKey", msgKey);

  // Store the new chain key and message key
  state->sendChainKey = std::move(newChain);
  state->send_message_keys[state->send_msg_num] = std::move(msgKey);
  if constexpr (printNormDebug) std::cerr << "[symmetricRatchetStep] msg_num=" << state->send_msg_num << std::endl;
  state->send_msg_num++;
  return true;
}

bool DoubleRatchet::receiveSymmetricRatchetStep(uint32_t msg_num) {
  const size_t OUTLEN = 64;
  std::vector<uint8_t> out(OUTLEN);

  // Use sendChainKey instead of recvChainKey for symmetric operation
  if (state->sendChainKey.empty()) {
    if constexpr (printNormDebug) std::cerr << "[receiveSymmetricRatchetStep] Error: sendChainKey is empty" << std::endl;
    return false;
  }

  if (state->sharedSecret.empty()) {
    if constexpr (printNormDebug) std::cerr << "[receiveSymmetricRatchetStep] Error: sharedSecret is empty" << std::endl;
    return false;
  }

  // Use same KDF parameters as in symmetricRatchetStep
  if (!kdfEnv->startKDF(state->sendChainKey, state->sharedSecret, "SendChainStep", out, OUTLEN)) {
    if constexpr (printNormDebug) std::cerr << "[receiveSymmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }

  std::vector<uint8_t> newChain(out.begin(), out.begin() + 32);
  std::vector<uint8_t> msgKey(out.begin() + 32, out.end());

  // Store the new chain key and message key
  state->sendChainKey = std::move(newChain);
  state->recv_message_keys[msg_num] = std::move(msgKey);

  return true;
}

bool DoubleRatchet::encryptMessage(const std::vector<uint8_t> &msg,
                                   std::vector<uint8_t> &cipher,
                                   std::vector<uint8_t> &tag,
                                   std::vector<uint8_t> &iv) {
  if constexpr (printNormDebug) std::cerr << "[encryptMessage] plaintext size=" << msg.size() << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[encryptMessage] plaintext", msg);

  // Perform symmetric ratchet step to generate the message key
  if (!symmetricRatchetStep()) return false;

  // Get the message key that was just created
  // Check if the key exists before accessing it
  if (state->send_message_keys.find(state->send_msg_num - 1) == state->send_message_keys.end()) {
    if constexpr (printNormDebug) std::cerr << "[encryptMessage] Error: Missing message key for send_msg_num=" << (state->send_msg_num - 1) <<
        std::endl;
    return false;
  }

  auto &key = state->send_message_keys[state->send_msg_num - 1];
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[encryptMessage] msgKey", key);

  // Set up encryption environment
  encryptionEnv->key = key;
  encryptionEnv->iv = iv;
  encryptionEnv->plaintext = msg;
  encryptionEnv->ciphertext = cipher;
  encryptionEnv->authTag = tag;

  if (!encryptionEnv->startEncryption()) {
    if constexpr (printNormDebug) std::cerr << "[encryptMessage] Encryption failed" << std::endl;
    return false;
  } else {
    if constexpr (printNormDebug) std::cerr << "[encryptMessage] Encryption succeeded" << std::endl;
    cipher = encryptionEnv->ciphertext;
    tag = encryptionEnv->authTag;
  }

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[encryptMessage] iv", encryptionEnv->iv);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[encryptMessage] cipher", cipher);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[encryptMessage] authTag", tag);
  return true;
}

bool DoubleRatchet::decryptMessage(const std::vector<uint8_t> &cipher,
                                   std::vector<uint8_t> &msg,
                                   const std::vector<uint8_t> &tag,
                                   const std::vector<uint8_t> &iv,
                                   uint32_t num) {
  if constexpr (printNormDebug) std::cerr << "[decryptMessage] cipher size=" << cipher.size() << " iv size=" << iv.size() << " tag size=" << tag.
      size() << " msgNum=" << num << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[decryptMessage] cipher", cipher);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[decryptMessage] authTag", tag);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[decryptMessage] iv", iv);

  // Generate message key if not already present
  if (state->recv_message_keys.find(num) == state->recv_message_keys.end()) {
    if constexpr (printNormDebug) std::cerr << "[decryptMessage] generating msgKey for num=" << num << std::endl;
    if (!receiveSymmetricRatchetStep(num)) {
      if constexpr (printNormDebug) std::cerr << "[decryptMessage] Failed to generate message key" << std::endl;
      return false;
    }
  }

  auto &key = state->recv_message_keys[num];
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[decryptMessage] msgKey", key);

  // Set up decryption environment
  decryptionEnv->key = key;
  decryptionEnv->iv = iv;
  decryptionEnv->ciphertext = cipher;
  decryptionEnv->authTag = tag;
  decryptionEnv->plaintext = msg;

  if (!decryptionEnv->startDecryption()) {
    if constexpr (printNormDebug) std::cerr << "[decryptMessage] Decryption failed" << std::endl;
    return false;
  } else {
    if constexpr (printNormDebug) std::cerr << "[decryptMessage] Decryption succeeded" << std::endl;
    msg = decryptionEnv->plaintext;
  }

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[decryptMessage] plaintext", msg);
  return true;
}

std::string DoubleRatchet::packEncMessage(const std::string &plaintext) {
  std::vector<uint8_t> msg(plaintext.begin(), plaintext.end()), cipher, tag;
  if constexpr (printNormDebug) std::cerr << "[packEncMessage] plaintext '" << plaintext << "'" << std::endl;

  KeyEnv keyEnv(KeyType::KeyIv);
  keyEnv.setKeyIvSizes(32, 12);
  std::vector<uint8_t> iv;
  std::vector<uint8_t> key;

  if (!keyEnv.startKeyIvGeneration(key, iv)) {
    return "";
  }

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[packEncMessage] generated iv", iv);

  if (!encryptMessage(msg, cipher, tag, iv)) {
    if constexpr (printNormDebug) std::cerr << "[packEncMessage] encryption failed" << std::endl;
    throw std::runtime_error("DoubleRatchet::packEncMessage: encryption failed");
  }

  // Ensure the current message number is used
  uint32_t currentMsgNum = state->send_msg_num - 1;

  if constexpr (printNormDebug) std::cout << "iv size=" << iv.size() << std::endl;
  if constexpr (printNormDebug) std::cout << "tag size=" << tag.size() << std::endl;
  if constexpr (printNormDebug) std::cout << "ownPubKey size=" << state->ownPubKey.size() << std::endl;
  if constexpr (printNormDebug) std::cout << "theirPubKey size=" << state->theirPubKey.size() << std::endl;

  if (iv.size() != 12 || tag.size() != 16 || state->ownPubKey.size() != 32 || state->theirPubKey.size() != 32) {
    if constexpr (printNormDebug) std::cerr << "[packEncMessage] Error: Invalid header element sizes" << std::endl;
    return "";
  }

  RatchetHeader hdr{
    iv, tag, state->ownPubKey, state->theirPubKey, currentMsgNum,
    static_cast<uint32_t>(msg.size())
  };

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[packEncMessage] hdr.iv", hdr.iv);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[packEncMessage] hdr.authTag", hdr.authTag);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[packEncMessage] hdr.SenderPubKey", hdr.SenderPubKey);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[packEncMessage] hdr.ReceiverPubKey", hdr.ReceiverPubKey);
  if constexpr (printNormDebug) std::cerr << "[packEncMessage] hdr.sendMessageNum=" << hdr.sendMessageNum << std::endl;
  if constexpr (printNormDebug) std::cerr << "[packEncMessage] hdr.messageLength=" << hdr.messageLength << std::endl;

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
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Error: Empty package" << std::endl;
    return "";
  }

  if (pkg.length() < 100) {
    // 12 (iv) + 16 (tag) + 32 (spk) + 32 (rpk) + 4 (num) + 4 (len)
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Error: Package too short, size=" << pkg.length() << std::endl;
    return "";
  }

  size_t pos = 0;
  auto read = [&](size_t n) {
    if (pos + n > pkg.length()) {
      if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Error: Trying to read beyond package bounds" << std::endl;
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
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Error: Package too short for message number and length" << std::endl;
    return "";
  }

  uint32_t num, len;
  std::memcpy(&num, pkg.data() + pos, 4);
  pos += 4;
  std::memcpy(&len, pkg.data() + pos, 4);
  pos += 4;

  if (pos > pkg.length()) {
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Error: Package too short for cipher data" << std::endl;
    return "";
  }

  std::vector<uint8_t> cipher(pkg.begin() + pos, pkg.end());
  if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] num=" << num << " len=" << len << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[unpackDecMessage] iv", iv);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[unpackDecMessage] tag", tag);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[unpackDecMessage] spk", spk);

  // Check if we need to do an asymmetric ratchet step
  if (spk != state->theirPubKey) {
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Performing asymmetric ratchet step" << std::endl;
    asymmetricRatchetStep(spk);
  }

  std::vector<uint8_t> out;
  if (!decryptMessage(cipher, out, tag, iv, num)) {
    if constexpr (printNormDebug) std::cerr << "[unpackDecMessage] Failed to decrypt message" << std::endl;
    return "";
  }

  return std::string(out.begin(), out.end());
}

bool DoubleRatchet::updateRootKey(const std::vector<uint8_t> &newPubKey) {
  if constexpr (printNormDebug) std::cerr << "[updateRootKey] newPubKey" << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[updateRootKey] newPubKey", newPubKey);

  // Ensure the new public key is set before the asymmetric ratchet step
  state->theirPubKey = newPubKey;

  return asymmetricRatchetStep(newPubKey);
}

RatchetState *DoubleRatchet::getState() const {
  if constexpr (printNormDebug) std::cerr << "[getState] send_msg_num=" << state->send_msg_num << " recv_msg_num=" << state->recv_msg_num <<
      std::endl;
  return state.get();
}

std::vector<uint8_t> DoubleRatchet::ownPubKey() const {
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[ownPubKey] ownPubKey", state->ownPubKey);
  return state->ownPubKey;
}

bool DoubleRatchet::asymmetricRatchetStep(const std::vector<uint8_t> &theirPub) {
  if constexpr (printNormDebug) std::cerr << "[asymmetricRatchetStep] starting" << std::endl;

  if (theirPub.empty()) {
    if constexpr (printNormDebug) std::cerr << "[asymmetricRatchetStep] Error: Empty theirPub" << std::endl;
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
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[asymmetricRatchetStep] sharedSecret", state->sharedSecret);

  // Derive new root key
  std::vector<uint8_t> newRoot(32);
  if (!kdfEnv->startKDF(state->rootKey, state->sharedSecret, "DH-Ratchet-Update", newRoot, 32)) {
    if constexpr (printNormDebug) std::cerr << "[asymmetricRatchetStep] KDF failed" << std::endl;
    return false;
  }
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[asymmetricRatchetStep] newRoot", newRoot);

  // Update state
  state->rootKey = std::move(newRoot);
  state->sendChainKey = state->rootKey;
  state->recvChainKey = state->rootKey;

  // Reset message counters for new chain
  state->send_msg_num = 0;
  state->recv_msg_num = 0;

  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[asymmetricRatchetStep] sendChainKey", state->sendChainKey);
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[asymmetricRatchetStep] recvChainKey", state->recvChainKey);
  return true;
}

bool DoubleRatchet::testOneSideDoubleRatchet() {
  if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] start" << std::endl;

  std::unique_ptr<KeyEnv> bobKeyEnv = std::make_unique<KeyEnv>(KeyType::X25519Keypair);
  bobKeyEnv->startKeyPairGeneration(true);
  auto bobPubKey = bobKeyEnv->getPublicRaw();

  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created Bob's KeyEnv and got public key" << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] bobPubKey", bobPubKey);

  DoubleRatchet alice(SessionType::DUO, ConstructType::INIT, nullptr, nullptr, bobPubKey);
  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created test alice" << std::endl;

  // Get Alice's public key
  auto alicePubKey = alice.ownPubKey();
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] alicePubKey", alicePubKey);

  // Create Bob with Alice's pubkey
  DoubleRatchet bob(SessionType::DUO, ConstructType::FOLLOWINIT, nullptr, std::move(bobKeyEnv), alicePubKey);
  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created test bob" << std::endl;

  // Now the shared secrets should match
  if (alice.getState()->sharedSecret == bob.getState()->sharedSecret) {
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] shared secrets match" << std::endl;
  } else {
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] shared secrets do not match" << std::endl;
    if constexpr (printHexDebug)
      Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] alice sharedSecret",
                                   alice.getState()->sharedSecret);
    if constexpr (printHexDebug)
      Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] bob sharedSecret",
                                   bob.getState()->sharedSecret);
    return false;
  }

  // Test encryption/decryption
  bool ok = true;
  for (size_t i = 0; i < testAmount; ++i) {
    std::string msg = "Test Nachricht: " + std::to_string(i);
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] encrypting message: " << msg << std::endl;

    try {
      auto pkg = alice.packEncMessage(msg);
      if (pkg == "") {
        if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
        ok = false;
        continue;
      }
      if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

      auto dec = bob.unpackDecMessage(pkg);
      if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;

      if (dec != msg) {
        if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Error: decrypted message does not match original" << std::endl;
        ok = false;
      } else {
        if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Success: message correctly encrypted and decrypted" << std::endl;
      }
    } catch (const std::exception &e) {
      if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Exception: " << e.what() << std::endl;
      ok = false;
    }
  }

  return ok;
}

bool DoubleRatchet::testMixedDoubleRatchet() {
  if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] start" << std::endl;

  std::unique_ptr<KeyEnv> bobKeyEnv = std::make_unique<KeyEnv>(KeyType::X25519Keypair);
  bobKeyEnv->startKeyPairGeneration(true);
  auto bobPubKey = bobKeyEnv->getPublicRaw();

  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created Bob's KeyEnv and got public key" << std::endl;
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] bobPubKey", bobPubKey);

  DoubleRatchet alice(SessionType::DUO, ConstructType::INIT, nullptr, nullptr, bobPubKey);
  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created test alice" << std::endl;

  // Get Alice's public key
  auto alicePubKey = alice.ownPubKey();
  if constexpr (printHexDebug) Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] alicePubKey", alicePubKey);

  // Create Bob with Alice's pubkey
  DoubleRatchet bob(SessionType::DUO, ConstructType::FOLLOWINIT, nullptr, std::move(bobKeyEnv), alicePubKey);
  if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] created test bob" << std::endl;

  // Now the shared secrets should match
  if (alice.getState()->sharedSecret == bob.getState()->sharedSecret) {
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] shared secrets match" << std::endl;
  } else {
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] shared secrets do not match" << std::endl;
    if constexpr (printHexDebug)
      Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] alice sharedSecret",
                                   alice.getState()->sharedSecret);
    if constexpr (printHexDebug)
      Converter::HexConverter::printBytesAsHex("[testDoubleRatchet] bob sharedSecret",
                                   bob.getState()->sharedSecret);
    return false;
  }

  // Test encryption/decryption
  bool ok = true;
  for (size_t i = 0; i < testAmount; ++i) {
    std::string msg = "Test Nachricht: " + std::to_string(i);
    if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] encrypting message: " << msg << std::endl;

    try {
      std::string dec;
      if (i % 2 == 0) {
        auto pkg = alice.packEncMessage(msg);
        if (pkg == "") {
          if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
          ok = false;
          continue;
        }
        if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

        dec = bob.unpackDecMessage(pkg);
        if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;
      } else {
        auto pkg = bob.packEncMessage(msg);
        if (pkg == "") {
          if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Error: empty package" << std::endl;
          ok = false;
          continue;
        }
        if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] encrypted message, package size: " << pkg.size() << std::endl;

        dec = alice.unpackDecMessage(pkg);
        if constexpr (printNormDebug) std::cout << "[testDoubleRatchet] decrypted message: " << dec << std::endl;
      }


      if (dec != msg) {
        if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Error: decrypted message does not match original" << std::endl;
        ok = false;
      } else {
        if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Success: message correctly encrypted and decrypted" << std::endl;
      }
    } catch (const std::exception &e) {
      if constexpr (printNormDebug) std::cerr << "[testDoubleRatchet] Exception: " << e.what() << std::endl;
      ok = false;
    }
  }

  return ok;
}
