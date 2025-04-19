#include "EncryptionEnv.h"

namespace Crypto {
  EncryptionEnv::EncryptionEnv(EncAlgorithm inAlgorithm) {
    algorithm = inAlgorithm;
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

      // Ensure memory is properly aligned
      std::unique_ptr<unsigned char[]> temp_key(new unsigned char[key.size()]);
      std::unique_ptr<unsigned char[]> temp_iv(new unsigned char[iv.size()]);

      if (RAND_bytes(temp_key.get(), static_cast<int>(key.size())) != 1) return false;
      if (RAND_bytes(temp_iv.get(), static_cast<int>(iv.size())) != 1) return false;

      std::copy(temp_key.get(), temp_key.get() + key.size(), key.data());
      std::copy(temp_iv.get(), temp_iv.get() + iv.size(), iv.data());

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
