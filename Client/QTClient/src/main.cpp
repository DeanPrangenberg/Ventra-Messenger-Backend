#include <QApplication>
#include <QPushButton>
#include <iostream>
#include "Crypto/Sync/EncryptionEnv.h"
#include "Crypto/Hash/HashingEnv.h"
#include "ThreadPool/ThreadPool.h"
#include "Crypto/KeyEnv/KeyEnv.h"

void test_encryption_hash() {
  Crypto::EncryptionEnv crypto(Crypto::EncAlgorithm::AES256);
  crypto.generateParameters();

  std::string plaintext = "Hello world!";
  crypto.plaintext.resize(plaintext.length());
  std::copy(plaintext.begin(), plaintext.end(), crypto.plaintext.begin());

  crypto.startEncryption();

  std::cout << "Ciphertext: ";
  for (const auto c: crypto.ciphertext) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  crypto.startDecryption();

  std::cout << "Plaintext: ";
  for (const auto c: crypto.plaintext) {
    std::cout << c;
  }
  std::cout << std::endl;

  Crypto::HashingEnv hasher(Crypto::HashAlgorithm::BLAKE2s256);

  hasher.plainData = crypto.plaintext;
  hasher.startHashing();

  std::cout << "Plain Hash Result: ";
  for (const auto c : hasher.hashValue ) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::endl;
  std::cout << std::dec;
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QPushButton button("Hello world!", nullptr);
  button.resize(200, 100);
  button.show();

  Crypto::KeyEnv keyEnv(Crypto::KeyType::X25519Keypair);
  keyEnv.startKeyPairGeneration();
  auto& keyPair = keyEnv.getKeyPair();

  for (const auto c: keyPair.getPublicRaw()) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  for (const auto c: keyPair.getPrivateRaw()) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  Crypto::KeyEnv keyEnv2(Crypto::KeyType::X25519Keypair);
  keyEnv2.startKeyPairGeneration(false, Crypto::KeyPairFormat::Raw, keyPair.getPublicRaw(), Crypto::KeyPairFormat::Raw, keyPair.getPrivateRaw());
  auto& keyPair2 = keyEnv2.getKeyPair();

  for (const auto c: keyPair2.getPublicRaw()) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  for (const auto c: keyPair2.getPrivateRaw()) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  return QApplication::exec();
}
