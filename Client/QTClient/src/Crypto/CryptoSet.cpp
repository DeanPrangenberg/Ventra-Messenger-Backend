#include "CryptoSet.h"
#include <memory>

namespace Crypto {
  CryptoSet::CryptoSet(Algorithm algorithm) {
    pAlgorithm = algorithm;
  }

  bool CryptoSet::encrypt() {
    if (!isValid()) return false;

    if (pAlgorithm == Algorithm::AES256) {
      return AES256::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), tag.data(), ciphertext.data());
    } else if (pAlgorithm == Algorithm::ChaCha20) {
      return ChaCha20::encrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), tag.data(),
                               ciphertext.data());
    }
    return false;
  }

  bool CryptoSet::decrypt() {
    if (!isValid()) return false;

    if (pAlgorithm == Algorithm::AES256) {
      return AES256::decrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), tag.data(), ciphertext.data());
    } else if (pAlgorithm == Algorithm::ChaCha20) {
      return ChaCha20::decrypt(plaintext.data(), plaintext.size(), key.data(), iv.data(), tag.data(),
                               ciphertext.data());
    }
    return false;
  }

  bool CryptoSet::generateParameters() {
    try {
      if (pAlgorithm == Algorithm::AES256) {
        key.resize(32);
        iv.resize(16);
        tag.resize(16);
        ciphertext.resize(plaintext.size() + 16);
      } else if (pAlgorithm == Algorithm::ChaCha20) {
        key.resize(32);
        iv.resize(12);
        tag.resize(16);
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

  bool CryptoSet::isValid() const {
    return !key.empty() && !iv.empty() && !tag.empty() && !ciphertext.empty();
  }
}
