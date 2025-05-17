//
// Created by deanprangenberg on 01.05.25.
//

#ifndef KDFENV_H
#define KDFENV_H

#include <vector>
#include <string>
#include <unordered_map>
#include "HKDF.h"

namespace Crypto {
  enum class KDFType {
    SHA3_512,
    SHA3_256,
    SHA2_512,
    SHA2_256
  };

  class KDFEnv {
  public:
    explicit KDFEnv(KDFType inAlgorithm);
    void changeAlgorithm(KDFType inAlgorithm);

    bool startKDF(const std::vector<uint8_t> &ikm, const std::vector<uint8_t> &salt, const std::string &info,
                        std::vector<uint8_t> &out,
                        size_t output_length);

  private:
    const EVP_MD *algo = nullptr;
  };
} // Crypto

#endif //KDFENV_H
