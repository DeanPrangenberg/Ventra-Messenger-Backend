//
// Created by deanprangenberg on 19.04.25.
//

#include "BLAKE2s256.h"

namespace Crypto {
  bool BLAKE2s256::hashData(const uint8_t *data, size_t size, uint8_t *hash, unsigned int *hashSize) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    try {
      if (!mdctx) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
      }

      if (EVP_DigestInit_ex(mdctx, EVP_blake2s256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize digest");
      }

      if (EVP_DigestUpdate(mdctx, data, size) != 1) {
        throw std::runtime_error("Failed to update digest");
      }

      if (EVP_DigestFinal_ex(mdctx, hash, hashSize) != 1) {
        throw std::runtime_error("Failed to finalize digest");
      }
    } catch (...) {
      EVP_MD_CTX_free(mdctx);
      return false;
    }

    EVP_MD_CTX_free(mdctx);

    return true;
  }

}
