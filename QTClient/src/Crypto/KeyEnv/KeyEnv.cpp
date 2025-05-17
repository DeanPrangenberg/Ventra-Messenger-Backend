#include "KeyEnv.h"

#include <iomanip>
#include <iostream>

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

  std::vector<uint8_t> KeyEnv::deriveSharedSecret(const std::vector<uint8_t> &peerPublic) const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->deriveSharedSecret(peerPublic);
    } else {
      throw std::logic_error("getPublicRaw only valid for X25519Keypair");
    }
  }

  std::vector<uint8_t> KeyEnv::getPublicRaw() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPublicRaw();
    } else {
      throw std::logic_error("getPublicRaw only valid for X25519Keypair");
    }
  }

  std::vector<uint8_t> KeyEnv::getPrivateRaw() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPrivateRaw();
    } else {
      throw std::logic_error("getPrivateRaw only valid for X25519Keypair");
    }
  }

  std::string KeyEnv::getPublicPem() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPublicPem();
    } else {
      throw std::logic_error("getPublicPem only valid for X25519Keypair");
    }
  }

  std::string KeyEnv::getPrivatePem() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPrivatePem();
    } else {
      throw std::logic_error("getPrivatePem only valid for X25519Keypair");
    }
  }

  std::string KeyEnv::getPublicBase64() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPublicBase64();
    } else {
      throw std::logic_error("getPublicBase64 only valid for X25519Keypair");
    }
  }

  std::string KeyEnv::getPrivateBase64() const {
    if (keyType_ == KeyType::X25519Keypair) {
      return keypair_->getPrivateBase64();
    } else {
      throw std::logic_error("getPrivateBase64 only valid for X25519Keypair");
    }
  }
  
} // namespace Crypto