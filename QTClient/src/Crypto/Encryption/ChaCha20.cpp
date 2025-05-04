//
// Created by deanprange on 3/16/25.
//

#include "ChaCha20.h"

namespace Crypto {
  bool ChaCha20::encrypt(const uint8_t *plaintext, size_t plaintext_len,
                       const uint8_t *key, const uint8_t *iv,
                       uint8_t *tag, uint8_t *ciphertext, int &ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    ciphertext_len = 0;

    // Setup ChaCha20-Poly1305 context
    if (1 != EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, key, iv)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Set associated data (optional, can be NULL if not needed)
    if (1 != EVP_EncryptUpdate(ctx, nullptr, &ciphertext_len, nullptr, 0)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Encrypt the plaintext
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, plaintext, plaintext_len)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Finalize encryption and get the tag
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &ciphertext_len)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Get the tag from the context
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, tag)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
  }

  bool ChaCha20::decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                         const uint8_t *key, const uint8_t *iv,
                         const uint8_t *tag, uint8_t *plaintext, int &plaintext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    plaintext_len = 0;

    // Setup ChaCha20-Poly1305 context
    if (1 != EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, key, iv)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Set associated data (optional, can be NULL if not needed)
    if (1 != EVP_DecryptUpdate(ctx, nullptr, &plaintext_len, nullptr, 0)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Decrypt the ciphertext
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, ciphertext, ciphertext_len)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Set the tag for verification
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, const_cast<uint8_t *>(tag))) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    // Finalize decryption and verify the tag
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + plaintext_len, &plaintext_len)) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
  }
} // Crypto
