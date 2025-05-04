//
// Created by deanprangenberg on 19.04.25.
//

#ifndef BLAKE2B512_H
#define BLAKE2B512_H

#include "../Encryption/EncryptionEnv.h"
#include <openssl/evp.h>

namespace Crypto {
  class HashingEnv;

  class BLAKE2b512 {
    friend class HashingEnv;
  private:
    static bool hashData(const uint8_t *data, size_t size, uint8_t *hash, unsigned int *hashSize);
  };
};


#endif //BLAKE2B512_H
