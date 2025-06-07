/*
 * Created by deanprange on 3/16/25.
 */

#include "AES256.h"
#include <openssl/evp.h>
#include <vector>
#include <cstring>
#include <iostream>

namespace Crypto {

  bool AES256::encrypt(const uint8_t *plaintext, size_t plaintext_len,
                       const uint8_t *key, const uint8_t *iv,
                       uint8_t *tag, uint8_t *ciphertext, int &ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
      std::cerr << "[AES256::encrypt] Error: Failed to create EVP_CIPHER_CTX" << std::endl;
      return false;
    }

    int len = 0;
    ciphertext_len = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_EncryptInit_ex failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_CIPHER_CTX_ctrl(SET_IVLEN) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_EncryptInit_ex(set key+iv) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, static_cast<int>(plaintext_len)) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_EncryptUpdate failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_EncryptFinal_ex failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    ciphertext_len += len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) {
      std::cerr << "[AES256::encrypt] Error: EVP_CIPHER_CTX_ctrl(GET_TAG) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
  }

  bool AES256::decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                       const uint8_t *key, const uint8_t *iv,
                       const uint8_t *tag, uint8_t *plaintext, int &plaintext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
      std::cerr << "[AES256::decrypt] Error: Failed to create EVP_CIPHER_CTX" << std::endl;
      return false;
    }

    int len = 0;
    plaintext_len = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
      std::cerr << "[AES256::decrypt] Error: EVP_DecryptInit_ex failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1) {
      std::cerr << "[AES256::decrypt] Error: EVP_CIPHER_CTX_ctrl(SET_IVLEN) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) {
      std::cerr << "[AES256::decrypt] Error: EVP_DecryptInit_ex(set key+iv) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, static_cast<int>(ciphertext_len)) != 1) {
      std::cerr << "[AES256::decrypt] Error: EVP_DecryptUpdate failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *)tag) != 1) {
      std::cerr << "[AES256::decrypt] Error: EVP_CIPHER_CTX_ctrl(SET_TAG) failed" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    if (ret <= 0) {
      std::cerr << "[AES256::decrypt] Error: EVP_DecryptFinal_ex failed (tag mismatch)" << std::endl;
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return true;
  }

} // namespace Crypto
