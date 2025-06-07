#ifndef KEYENV_H
#define KEYENV_H

#include <vector>
#include <stdexcept>
#include <optional>
#include "RandomVec.h"
#include "X25519KeyPair.h"

namespace Crypto {
  enum class KeyType {
    X25519Keypair,
    KeyIv,
  };

  class KeyEnv {
  public:
    explicit KeyEnv(KeyType inKeyType);

    // Konfiguration für Key/IV
    void setKeyIvSizes(size_t keyLength, size_t ivLength);
    bool startKeyIvGeneration(std::vector<uint8_t> &key,
                              std::vector<uint8_t> &iv);

    // Erzeugung oder Laden von X25519-Schlüsselpaar
    bool startKeyPairGeneration(bool generate = true,
                                KeyPairFormat pubFormat = KeyPairFormat::None,
                                const std::vector<uint8_t> &pubRaw = {},
                                KeyPairFormat privFormat = KeyPairFormat::None,
                                const std::vector<uint8_t> &privRaw = {});

    std::vector<uint8_t> deriveSharedSecret(const std::vector<uint8_t> &peerPublic) const;

    std::vector<uint8_t> getPublicRaw() const;
    std::vector<uint8_t> getPrivateRaw() const;
    std::string getPublicPem() const;
    std::string getPrivatePem() const;
    std::string getPublicBase64() const;
    std::string getPrivateBase64() const;

  private:
    KeyType keyType_;

    // Für KeyIv
    bool keyIvSizeSet_;
    size_t keyLen_;
    size_t ivLen_;

    // Für X25519
    std::optional<X25519KeyPair> keypair_;
  };
} // namespace Crypto

#endif // KEYENV_H