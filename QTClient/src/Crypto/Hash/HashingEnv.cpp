#include "HashingEnv.h"

namespace Crypto {
  HashingEnv::HashingEnv(HashAlgorithm inAlgorithm) {
    algorithm = inAlgorithm;
  }

  bool HashingEnv::generateParameters() {
    if (algorithm == HashAlgorithm::BLAKE2b512) {
      hashValue.resize(64);
    } else if (algorithm == HashAlgorithm::BLAKE2s256) {
      hashValue.resize(32);
    } else {
      return false;
    }
    return true;
  }

  bool HashingEnv::startHashing() {
    if (plainData.empty()) {
      return false;
    }

    generateParameters();

    if (algorithm == HashAlgorithm::BLAKE2b512) {
      unsigned int hashSize = 0;
      if (BLAKE2b512::hashData(plainData.data(), plainData.size(), hashValue.data(), &hashSize)) {
        return true;
      }
    } else if (algorithm == HashAlgorithm::BLAKE2s256) {
      unsigned int hashSize = 0;
      if (BLAKE2s256::hashData(plainData.data(), plainData.size(), hashValue.data(), &hashSize)) {
        return true;
      }
    }

    return false;
  }

  bool HashingEnv::isValid() const {
    return !plainData.empty();
  }
}
