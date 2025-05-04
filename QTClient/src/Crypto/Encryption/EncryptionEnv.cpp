#include "EncryptionEnv.h"
#include <iostream>
#include <iomanip>

namespace Crypto {

  static void printHex(const std::string &label, const std::vector<uint8_t> &data) {
    std::cerr << label << " [" << data.size() << "] = ";
    for (const auto b : data)
      std::cerr << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    std::cerr << std::dec << std::endl;
  }

  EncryptionEnv::EncryptionEnv(EncAlgorithm inAlgorithm) {
    algorithm = inAlgorithm;
    generateParameters();
  }

  bool EncryptionEnv::startEncryption() {
    if (!isValid()) {
      throw std::logic_error("[Crypto::startEncryption] Invalid encryption parameters");
    }

    ciphertext.resize(plaintext.size() + 16); // AES-GCM needs space for tag

    printHex("PLAINTEXT", plaintext);
    printHex("KEY", key);
    printHex("IV", iv);

    bool result = false;
    int ciphertext_len = 0;
    if (algorithm == EncAlgorithm::AES256) {
      authTag.resize(16);
      result = AES256::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(),
                               authTag.data(), ciphertext.data(), ciphertext_len);
    } else if (algorithm == EncAlgorithm::ChaCha20) {
      authTag.resize(16);
      result = ChaCha20::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(),
                                 authTag.data(), ciphertext.data(), ciphertext_len);
    }


    if (result) {
      ciphertext.resize(ciphertext_len);
      printHex("ENCRYPTED TEXT", ciphertext);
    } else {
      std::cerr << "[ERROR] ENcryption failed: authentication tag mismatch or data corrupt." << std::endl;
    }
    return result;
  }

  bool EncryptionEnv::startDecryption() {
    if (!isValid()) {
      throw std::logic_error("[Crypto::startDecryption] Invalid encryption parameters");
    }

    plaintext.resize(ciphertext.size()); // Max size of decrypted data

    printHex("CIPHERTEXT (BEFORE DECRYPT)", ciphertext);
    printHex("KEY", key);
    printHex("IV", iv);
    printHex("AUTH TAG", authTag);

    bool result = false;
    int plaintext_len = 0;
    if (algorithm == EncAlgorithm::AES256) {
      result = AES256::decrypt(ciphertext.data(), ciphertext.size(), key.data(), iv.data(),
                               authTag.data(), plaintext.data(), plaintext_len);
    } else if (algorithm == EncAlgorithm::ChaCha20) {
      result = ChaCha20::decrypt(ciphertext.data(), ciphertext.size(), key.data(), iv.data(),
                                 authTag.data(), plaintext.data(), plaintext_len);
    }

    if (result) {
      plaintext.resize(plaintext_len);
      printHex("DECRYPTED TEXT", plaintext);
    } else {
      std::cerr << "[ERROR] Decryption failed: authentication tag mismatch or data corrupt." << std::endl;
    }

    return result;
  }

  bool EncryptionEnv::generateParameters() {
    try {
      key.resize(32);
      iv.resize(12);
      authTag.clear();  // gets filled after encryption
      ciphertext.clear();

      KeyEnv keyEnv(KeyType::KeyIv);
      keyEnv.startKeyIvGeneration(key, iv);
      return true;
    } catch (const std::exception &e) {
      std::cerr << "[ERROR] Key/IV generation failed: " << e.what() << std::endl;
      return false;
    }
  }

  bool EncryptionEnv::isValid() const {
    return (algorithm == EncAlgorithm::AES256 || algorithm == EncAlgorithm::ChaCha20)
           && !key.empty() && !iv.empty();
  }
}
