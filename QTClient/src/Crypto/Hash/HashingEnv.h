//
// Created by deanprange on 3/16/25.
//

#ifndef HASHINGENV_H
#define HASHINGENV_H

#include <vector>
#include "BLAKE2b512.h"
#include "BLAKE2s256.h"

namespace Crypto {
  enum class HashAlgorithm {
    BLAKE2b512,
    BLAKE2s256
  };

  class HashingEnv {
  public:
    explicit HashingEnv(HashAlgorithm inAlgorithm);

    std::vector<uint8_t> plainData;
    std::vector<uint8_t> hashValue;

    bool startHashing();

  private:
    bool isValid() const;

    bool generateParameters();

    HashAlgorithm algorithm;
  };
}

#endif //HASHINGENV_H
