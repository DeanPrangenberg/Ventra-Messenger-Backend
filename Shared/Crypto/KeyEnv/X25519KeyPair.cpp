#include "X25519KeyPair.h"

namespace Crypto {
  // Generierung
  void X25519KeyPair::GenerateNewKeyPair() {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
    if (!ctx) throw OpenSSLError("Context creation failed");
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Keygen init failed");
    }
    EVP_PKEY *p = nullptr;
    if (EVP_PKEY_keygen(ctx, &p) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Key generation failed");
    }
    key_.reset(p);
    EVP_PKEY_CTX_free(ctx);
  }

  // Rohdaten
  std::vector<uint8_t> X25519KeyPair::getPublicRaw() const {
    size_t len = 0;
    if (EVP_PKEY_get_raw_public_key(key_.get(), nullptr, &len) != 1) throw OpenSSLError("Get public length failed");
    std::vector<uint8_t> buf(len);
    if (EVP_PKEY_get_raw_public_key(key_.get(), buf.data(), &len) != 1) throw OpenSSLError("Get public data failed");
    return buf;
  }

  std::vector<uint8_t> X25519KeyPair::getPrivateRaw() const {
    size_t len = 0;
    if (EVP_PKEY_get_raw_private_key(key_.get(), nullptr, &len) != 1) throw OpenSSLError("Get private length failed");
    std::vector<uint8_t> buf(len);
    if (EVP_PKEY_get_raw_private_key(key_.get(), buf.data(), &len) != 1) throw OpenSSLError("Get private data failed");
    return buf;
  }

  // PEM
  std::string X25519KeyPair::getPublicPem() const {
    BIO *bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(bio, key_.get())) {
      BIO_free(bio);
      throw OpenSSLError("Write public PEM failed");
    }
    return pemFromBio(bio);
  }

  std::string X25519KeyPair::getPrivatePem() const {
    BIO *bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PrivateKey(bio, key_.get(), nullptr, nullptr, 0, nullptr, nullptr)) {
      BIO_free(bio);
      throw OpenSSLError("Write private PEM failed");
    }
    return pemFromBio(bio);
  }

  // Base64
  std::string X25519KeyPair::getPublicBase64() const { return encodeBase64(getPublicRaw()); }
  std::string X25519KeyPair::getPrivateBase64() const { return encodeBase64(getPrivateRaw()); }

  // Gemeinsames Geheimnis
  std::vector<uint8_t> X25519KeyPair::deriveSharedSecret(const std::vector<uint8_t> &peerPublic) const {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key_.get(), nullptr);
    if (!ctx) throw OpenSSLError("Context creation failed");

    if (EVP_PKEY_derive_init(ctx) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Derive init failed");
    }

    auto peer = loadFromRaw(peerPublic, true);
    if (EVP_PKEY_derive_set_peer(ctx, peer.get()) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Set peer failed");
    }

    size_t len = 0;
    if (EVP_PKEY_derive(ctx, nullptr, &len) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Size derive failed");
    }

    // X25519 sollte immer 32 Bytes liefern
    if (len != 32) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Unexpected shared secret length: " + std::to_string(len));
    }

    std::vector<uint8_t> secret(len);
    if (EVP_PKEY_derive(ctx, secret.data(), &len) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      throw OpenSSLError("Secret derive failed");
    }

    // Sicherstellen, dass Vektor die tatsächliche Länge widerspiegelt
    secret.resize(len);

    EVP_PKEY_CTX_free(ctx);
    return secret;
  }


  // Lader
  X25519KeyPair::PKeyPtr X25519KeyPair::loadFromRaw(const std::vector<uint8_t> &d, bool pub) {
    EVP_PKEY *p = pub
                    ? EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr, d.data(), d.size())
                    : EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr, d.data(), d.size());
    if (!p) throw OpenSSLError("Invalid raw key");
    return PKeyPtr(p);
  }

  X25519KeyPair::PKeyPtr X25519KeyPair::loadFromPem(const std::string &pem, bool pub) {
    BIO *bio = BIO_new_mem_buf(pem.data(), pem.size());
    EVP_PKEY *p = pub
                    ? PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr)
                    : PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!p) throw OpenSSLError("Invalid PEM");
    return PKeyPtr(p);
  }

  X25519KeyPair::PKeyPtr X25519KeyPair::loadFromBase64(const std::string &b64, bool pub) {
    auto raw = decodeBase64(b64);
    return loadFromRaw(raw, pub);
  }

  // Base64 Hilfs
  std::string X25519KeyPair::encodeBase64(const std::vector<uint8_t> &data) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bio = BIO_push(b64, mem);
    if (BIO_write(bio, data.data(), data.size()) <= 0 || BIO_flush(bio) <= 0) {
      BIO_free_all(bio);
      throw OpenSSLError("Base64 encode failed");
    }
    return pemFromBio(mem);
  }

  std::vector<uint8_t> X25519KeyPair::decodeBase64(const std::string &b64) {
    BIO *b64f = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(b64.data(), b64.size());
    BIO_set_flags(b64f, BIO_FLAGS_BASE64_NO_NL);
    BIO *bio = BIO_push(b64f, mem);
    std::vector<uint8_t> buf(b64.size());
    int len = BIO_read(bio, buf.data(), buf.size());
    BIO_free_all(bio);
    if (len <= 0) throw OpenSSLError("Base64 decode failed");
    buf.resize(len);
    return buf;
  }

  std::string X25519KeyPair::pemFromBio(BIO *bio) {
    char *data;
    long len = BIO_get_mem_data(bio, &data);
    std::string res(data, len);
    BIO_free(bio);
    return res;
  }
} // namespace Crypto
