//
// Created by deanprange on 3/16/25.
//

#ifndef CRYPTO_H
#define CRYPTO_H

#include <cstdint>
#include <memory>
#include <vector>
#include <openssl/rand.h>
#include "ChaCha20.h"
#include "AES256.h"

namespace Crypto {
  enum class Algorithm {
    AES256,
    ChaCha20
  };

  class CryptoSet {
  public:
    explicit CryptoSet(Algorithm algorithm);
    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> tag;
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> plaintext;

    bool encrypt();
    bool decrypt();
    bool generateParameters();

  private:
    Algorithm pAlgorithm;
    bool isValid() const;
  };
}

#endif //CRYPTO_H
