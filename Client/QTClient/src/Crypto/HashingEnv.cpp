#include "HashingEnv.h"

namespace Crypto {
  HashingEnv::HashingEnv(HashAlgorithm inAlgorithm) {
    algorithm = inAlgorithm;
  }

  bool HashingEnv::generateParameters() {
    if (algorithm == HashAlgorithm::Blake2) {
      hashValue.resize(64);
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

    if (algorithm == HashAlgorithm::Blake2) {
      unsigned int hashSize = 0;
      if (Blake2::hashData(plainData.data(), plainData.size(), hashValue.data(), &hashSize)) {
        return true;
      }
    }

    return false;
  }

  bool HashingEnv::isValid() const {
    return !plainData.empty();
  }
}
