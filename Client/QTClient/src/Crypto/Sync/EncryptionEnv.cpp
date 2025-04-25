#include "EncryptionEnv.h"

namespace Crypto {
  EncryptionEnv::EncryptionEnv(EncAlgorithm inAlgorithm) {
    algorithm = inAlgorithm;
    generateParameters();
  }

  bool EncryptionEnv::startEncryption() {
    if (!isValid()) {
      return false;
    }

    if (algorithm == EncAlgorithm::AES256) {
      return AES256::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), authTag.data(),
                             ciphertext.data());
    } else if (algorithm == EncAlgorithm::ChaCha20) {
      return ChaCha20::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), authTag.data(),
                               ciphertext.data());
    }
    return false;
  }

  bool EncryptionEnv::startDecryption() {
    if (!isValid()) {
      return false;
    }

    if (algorithm == EncAlgorithm::AES256) {
      return AES256::decrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), authTag.data(),
                             ciphertext.data());
    } else if (algorithm == EncAlgorithm::ChaCha20) {
      return ChaCha20::decrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), authTag.data(),
                               ciphertext.data());
    }
    return false;
  }

  bool EncryptionEnv::generateParameters() {
    try {
      if (algorithm == EncAlgorithm::AES256) {
        key.resize(32);
        iv.resize(16);
        authTag.resize(16);
        ciphertext.resize(plaintext.size() + 16);
      } else if (algorithm == EncAlgorithm::ChaCha20) {
        key.resize(32);
        iv.resize(12);
        authTag.resize(16);
        ciphertext.resize(plaintext.size() + 16);
      } else {
        return false;
      }

      KeyEnv keyEnv(KeyType::KeyIv);
      keyEnv.startKeyIvGeneration(key, iv);
      return true;
    } catch (const std::exception &) {
      return false;
    }
  }

  bool EncryptionEnv::isValid() const {
    if (algorithm == EncAlgorithm::AES256 || algorithm == EncAlgorithm::ChaCha20) {
      return !key.empty() && !iv.empty() && !authTag.empty() && !plaintext.empty();
    }

    return false;
  }
}
