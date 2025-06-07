//
// Created by deanprangenberg on 01.05.25.
//

#include "KDFEnv.h"

namespace Crypto {
  KDFEnv::KDFEnv(KDFType inAlgorithm) {
    changeAlgorithm(inAlgorithm);
  }

  void KDFEnv::changeAlgorithm(KDFType inAlgorithm) {
    switch (inAlgorithm) {
      case KDFType::SHA3_512:
        algo = EVP_sha3_512(); // Kein Cast benötigt
        break;
      case KDFType::SHA3_256:
        algo = EVP_sha3_256();
        break;
      case KDFType::SHA2_512:
        algo = EVP_sha512();
        break;
      case KDFType::SHA2_256:
        algo = EVP_sha256();
        break;
      default:
        throw std::invalid_argument("Unknown KDF algorithm");
    }
  }

  bool KDFEnv::startKDF(const std::vector<uint8_t> &ikm, const std::vector<uint8_t> &salt, const std::string &info,
                        std::vector<uint8_t> &out,
                        size_t output_length) {
    if (algo == nullptr) {
      return false;
    }

    if (ikm.empty() || output_length == 0 || salt.empty() || info.empty() || out.empty()) {
      return false;
    }

    out.resize(output_length);

    out = HKDF::derive(algo, ikm, salt, info, output_length);
    return true;
  }
} // Crypto
