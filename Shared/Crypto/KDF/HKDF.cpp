//
// Created by deanprangenberg on 01.05.25.
//

#include "HKDF.h"
#include <openssl/err.h>
#include <openssl/kdf.h>
#include <memory>

namespace Crypto {
  std::string HKDF::getOpenSSLError() {
    char err_buf[256];
    ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
    return std::string(err_buf);
  }

  struct EVP_PKEY_CTX_deleter {
    void operator()(EVP_PKEY_CTX *ctx) const {
      EVP_PKEY_CTX_free(ctx);
    }
  };

  using EVP_PKEY_CTX_ptr = std::unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_deleter>;

  std::vector<uint8_t> HKDF::derive(
    const EVP_MD *digest,
    const std::vector<uint8_t> &ikm,
    const std::vector<uint8_t> &salt,
    const std::string &info,
    size_t output_length
  ) {

    std::vector<uint8_t> infoVec(info.begin(), info.end());

    EVP_PKEY_CTX *raw_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (!raw_ctx) {
      throw std::runtime_error("EVP_PKEY_CTX_new_id failed: " + getOpenSSLError());
    }
    EVP_PKEY_CTX_ptr ctx(raw_ctx);

    if (EVP_PKEY_derive_init(ctx.get()) <= 0) {
      throw std::runtime_error("EVP_PKEY_derive_init failed: " + getOpenSSLError());
    }

    if (EVP_PKEY_CTX_set_hkdf_md(ctx.get(), digest) <= 0) {
      throw std::runtime_error("EVP_PKEY_CTX_set_hkdf_md failed: " + getOpenSSLError());
    }

    if (EVP_PKEY_CTX_set_hkdf_mode(ctx.get(), EVP_PKEY_HKDEF_MODE_EXTRACT_AND_EXPAND) <= 0) {
      throw std::runtime_error("EVP_PKEY_CTX_set_hkdf_mode failed: " + getOpenSSLError());
    }

    if (!salt.empty()) {
      if (EVP_PKEY_CTX_set1_hkdf_salt(ctx.get(), salt.data(), salt.size()) <= 0) {
        throw std::runtime_error("EVP_PKEY_CTX_set1_hkdf_salt failed: " + getOpenSSLError());
      }
    }

    if (EVP_PKEY_CTX_set1_hkdf_key(ctx.get(), ikm.data(), ikm.size()) <= 0) {
      throw std::runtime_error("EVP_PKEY_CTX_set1_hkdf_key failed: " + getOpenSSLError());
    }

    if (!info.empty()) {
      if (EVP_PKEY_CTX_add1_hkdf_info(ctx.get(), infoVec.data(), infoVec.size()) <= 0) {
        throw std::runtime_error("EVP_PKEY_CTX_add1_hkdf_info failed: " + getOpenSSLError());
      }
    }

    std::vector<uint8_t> out(output_length);
    size_t outlen = output_length;

    if (EVP_PKEY_derive(ctx.get(), out.data(), &outlen) <= 0) {
      throw std::runtime_error("EVP_PKEY_derive failed: " + getOpenSSLError());
    }

    if (outlen != output_length) {
      throw std::runtime_error("HKDF output length mismatch");
    }

    return out;
  }
}
