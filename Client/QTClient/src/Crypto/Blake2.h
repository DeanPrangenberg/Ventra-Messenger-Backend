//
// Created by deanprangenberg on 19.04.25.
//

#ifndef BLAKE2_H
#define BLAKE2_H

#include "EncryptionEnv.h"
#include <openssl/evp.h>

namespace Crypto {
  class EncryptionEnv;

  class Blake2 {
    friend class EncryptionEnv;
  public:

    static bool hashData(const uint8_t *data, size_t size, uint8_t *hash, unsigned int *hashSize);
  };
};


#endif //BLAKE2_H
