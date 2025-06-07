//
// Created by deanprange on 3/16/25.
//

#ifndef ENCRYPTIONENV_H
#define ENCRYPTIONENV_H

#include <vector>
#include "ChaCha20.h"
#include "AES256.h"
#include "../KeyEnv/KeyEnv.h"
#include <memory>

namespace Crypto {
  enum class EncAlgorithm {
    AES256,
    ChaCha20
  };

  class EncryptionEnv {
  public:
    explicit EncryptionEnv(EncAlgorithm algorithm);

    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> authTag;
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> plaintext;

    bool startEncryption();

    bool startDecryption();

    bool generateParameters();

  private:
    bool isValid() const;

    EncAlgorithm algorithm;
  };
}

#endif //ENCRYPTIONENV_H