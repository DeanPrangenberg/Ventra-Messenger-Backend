#include "KeyEnv.h"

namespace Crypto {

  KeyEnv::KeyEnv(KeyType inKeyType)
    : keyType_(inKeyType), keyIvSizeSet_(false) {
    if (keyType_ != KeyType::KeyIv && keyType_ != KeyType::X25519Keypair) {
      throw std::invalid_argument("Invalid KeyType");
    }
  }

  void KeyEnv::setKeyIvSizes(size_t keyLength, size_t ivLength) {
    if (keyType_ != KeyType::KeyIv)
      throw std::logic_error("setKeyIvSizes only valid for KeyIv type");
    keyLen_ = keyLength;
    ivLen_ = ivLength;
    keyIvSizeSet_ = true;
  }

  bool KeyEnv::startKeyIvGeneration(std::vector<uint8_t> &key,
                                    std::vector<uint8_t> &iv) {
    if (keyType_ != KeyType::KeyIv || !keyIvSizeSet_)
      return false;
    RandomVec::generateKeyIv(key, keyLen_, iv, ivLen_);
    return true;
  }

  bool KeyEnv::startKeyPairGeneration(bool generate,
                                      KeyPairFormat pubFormat,
                                      const std::vector<uint8_t> &pubRaw,
                                      KeyPairFormat privFormat,
                                      const std::vector<uint8_t> &privRaw) {
    if (keyType_ != KeyType::X25519Keypair)
      return false;
    // Erzeugung/Laden über X25519KeyPair
    keypair_.emplace(generate, pubFormat, pubRaw, privFormat, privRaw);
    return true;
  }

  const X25519KeyPair &KeyEnv::getKeyPair() const {
    if (!keypair_)
      throw std::logic_error("Key pair not generated or loaded");
    return *keypair_;
  }

} // namespace Crypto