//
// Created by deanprangenberg on 01.05.25.
//

#ifndef HKDF_H
#define HKDF_H

#include <vector>
#include <stdexcept>
#include <openssl/evp.h>

namespace Crypto {
  class KDFEnv;

  class HKDF {
    friend class KDFEnv;

  private:
    static std::vector<uint8_t> derive(
      const EVP_MD *digest,
      const std::vector<uint8_t> &ikm,
      const std::vector<uint8_t> &salt,
      const std::string &info,
      size_t output_length
    );

    static std::string getOpenSSLError();
  };
} ;

#endif //HKDF_H
