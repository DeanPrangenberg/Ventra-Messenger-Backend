/*
 * Created by deanprange on 3/16/25.
 */

#include "AES256.h"
#include <openssl/evp.h>
#include <vector>
#include <cstring>

namespace Crypto {
  bool AES256::encrypt(const uint8_t *plaintext, size_t plaintext_len,
                       const uint8_t *key, const uint8_t *iv,
                       uint8_t *tag, uint8_t *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    int len = 0;
    int ciphertext_len = 0;

    // Setup AES-256-GCM context
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Encrypt the plaintext
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, static_cast<int>(plaintext_len))) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    ciphertext_len += len;

    // Get authentication tag
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
  }

  bool AES256::decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                       const uint8_t *key, const uint8_t *iv,
                       const uint8_t *tag, uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    int len = 0;
    int plaintext_len = 0;

    // Setup AES-256-GCM context
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Decrypt the ciphertext
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, static_cast<int>(ciphertext_len))) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    plaintext_len = len;

    // Set authentication tag for verification
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void *) tag)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Finalize decryption
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    // Check authentication tag validity
    return ret > 0;
  }
} // namespace Crypto
