//
// Created by deanprangenberg on 19.04.25.
//

#ifndef RANDOMVEC_H
#define RANDOMVEC_H

#include <vector>
#include <memory>
#include <openssl/rand.h>
#include <openssl/err.h>

namespace Crypto {
  class KeyEnv;
  class RandomVec {
    friend class KeyEnv;
  private:
    static bool generateKeyIv(std::vector<uint8_t> &key, size_t keyLen,
                              std::vector<uint8_t> &iv, size_t ivLen);
  };
} // Crypto

#endif //RANDOMVEC_H
