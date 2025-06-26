#ifndef X25519KEYPAIR_H
#define X25519KEYPAIR_H

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace Crypto {
  enum class KeyPairFormat {
    None,
    Raw,
    Pem,
    Base64
  };

  class OpenSSLError : public std::runtime_error {
  public:
    explicit OpenSSLError(const std::string &msg)
      : std::runtime_error(msg + ": " + getError()) {
    }

  private:
    static std::string getError() {
      char buf[256];
      ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
      return buf;
    }
  };

  template<typename T>
  concept AllowedKeyType =
      std::is_same_v<T, std::string> ||
      std::is_same_v<T, std::vector<uint8_t> >;

  class X25519KeyPair {
  public:
    // Flexibler Haupt-Konstruktor:
    // - generate=true: neues Schlüsselpaar
    // - generate=false: lädt pub/priv gemäß Formaten oder ignoriert None
    template<AllowedKeyType T1 = std::vector<uint8_t>,
      AllowedKeyType T2 = std::vector<uint8_t> >
    explicit X25519KeyPair(
      bool generate,
      KeyPairFormat pubFormat = KeyPairFormat::None,
      const T1 &pubKey = {},
      KeyPairFormat privFormat = KeyPairFormat::None,
      const T2 &privKey = {}
    ) {
      if (generate) {
        GenerateNewKeyPair();
        return;
      }
      // Laden öffentlicher Schlüssel
      if (pubFormat != KeyPairFormat::None) {
        switch (pubFormat) {
          case KeyPairFormat::Raw:
            if constexpr (std::is_same_v<T1, std::vector<uint8_t> >) key_ = loadFromRaw(pubKey, true);
            else throw std::invalid_argument("Raw public key requires vector<uint8_t>");
            break;
          case KeyPairFormat::Pem:
            if constexpr (std::is_same_v<T1, std::string>) key_ = loadFromPem(pubKey, true);
            else throw std::invalid_argument("PEM public key requires string");
            break;
          case KeyPairFormat::Base64:
            if constexpr (std::is_same_v<T1, std::string>) key_ = loadFromBase64(pubKey, true);
            else throw std::invalid_argument("Base64 public key requires string");
            break;
          default: break;
        }
      }
      // Laden privater Schlüssel
      if (privFormat != KeyPairFormat::None) {
        switch (privFormat) {
          case KeyPairFormat::Raw:
            if constexpr (std::is_same_v<T2, std::vector<uint8_t> >) key_ = loadFromRaw(privKey, false);
            else throw std::invalid_argument("Raw private key requires vector<uint8_t>");
            break;
          case KeyPairFormat::Pem:
            if constexpr (std::is_same_v<T2, std::string>) key_ = loadFromPem(privKey, false);
            else throw std::invalid_argument("PEM private key requires string");
            break;
          case KeyPairFormat::Base64:
            if constexpr (std::is_same_v<T2, std::string>) key_ = loadFromBase64(privKey, false);
            else throw std::invalid_argument("Base64 private key requires string");
            break;
          default: break;
        }
      }
    }

    // Convenience-Overloads für nur einen Schlüssel
    template<AllowedKeyType T>
    explicit X25519KeyPair(KeyPairFormat pubFormat, const T &pubKey)
      : X25519KeyPair(false, pubFormat, pubKey, KeyPairFormat::None, {}) {
    }

    template<AllowedKeyType T>
    explicit X25519KeyPair(const T &privKey, KeyPairFormat privFormat)
      : X25519KeyPair(false, KeyPairFormat::None, std::vector<uint8_t>(), privFormat, privKey) {
    }

    // Schlüsselzugriff
    std::vector<uint8_t> getPublicRaw() const;

    std::vector<uint8_t> getPrivateRaw() const;

    std::string getPublicPem() const;

    std::string getPrivatePem() const;

    std::string getPublicBase64() const;

    std::string getPrivateBase64() const;

    // Gemeinsames Geheimnis
    std::vector<uint8_t> deriveSharedSecret(const std::vector<uint8_t> &peerPublic) const;

  private:
    struct PKeyDeleter {
      void operator()(EVP_PKEY *p) const { EVP_PKEY_free(p); }
    };

    using PKeyPtr = std::unique_ptr<EVP_PKEY, PKeyDeleter>;
    PKeyPtr key_;

    // Intern: Schlüssellader
    static PKeyPtr loadFromRaw(const std::vector<uint8_t> &data, bool pub);

    static PKeyPtr loadFromPem(const std::string &pem, bool pub);

    static PKeyPtr loadFromBase64(const std::string &b64, bool pub);

    static std::string encodeBase64(const std::vector<uint8_t> &data);

    static std::vector<uint8_t> decodeBase64(const std::string &b64);

    static std::string pemFromBio(BIO *bio);

    // Schlüsselgenerierung
    void GenerateNewKeyPair();
  };
} // namespace Crypto

#endif // X25519KEYPAIR_H
