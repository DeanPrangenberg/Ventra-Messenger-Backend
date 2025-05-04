//
// Created by deanprangenberg on 19.04.25.
//

#include "RandomVec.h"

namespace Crypto {
  bool RandomVec::generateKeyIv(std::vector<uint8_t> &key, const size_t keyLen,
                                std::vector<uint8_t> &iv, const size_t ivLen) {

    key.resize(keyLen);
    iv.resize(ivLen);

    if (RAND_priv_bytes(key.data(), keyLen) != 1 || RAND_priv_bytes(iv.data(), ivLen) != 1) {
      return false;
    }
    return true;
  }
} // Crypto
