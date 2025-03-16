//
// Created by deanprange on 3/16/25.
//

#ifndef AES256_H
#define AES256_H

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iostream>
#include <cstring>

namespace Crypto {
  class CryptoSet;

  class AES256 {
    friend class CryptoSet;
  private:
    static bool encrypt(const uint8_t *plaintext, size_t plaintext_len, const uint8_t *key, const uint8_t *iv,
                        uint8_t *tag, uint8_t *ciphertext);

    static bool decrypt(const uint8_t *ciphertext, size_t ciphertext_len, const uint8_t *key, const uint8_t *iv,
                        const uint8_t *tag, uint8_t *plaintext);
  };
} // Crypto

#endif //AES256_H
