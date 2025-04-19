#include <QApplication>
#include <QPushButton>
#include <iostream>
#include "Crypto/EncryptionEnv.h"
#include "Crypto/HashingEnv.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QPushButton button("Hello world!", nullptr);
  button.resize(200, 100);
  button.show();

  Crypto::EncryptionEnv crypto(Crypto::EncAlgorithm::AES256);


  std::string plaintext = "Hello world!";
  crypto.plaintext.resize(plaintext.length());
  std::copy(plaintext.begin(), plaintext.end(), crypto.plaintext.begin());

  crypto.generateParameters();
  crypto.startEncryption();

  std::cout << "Ciphertext: ";
  for (const auto c: crypto.ciphertext) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::dec << std::endl;

  // Store ciphertext for decryption
  auto encrypted = crypto.ciphertext;
  crypto.ciphertext = encrypted;

  crypto.startDecryption();

  std::cout << "Plaintext: ";
  for (const auto c: crypto.plaintext) {
    std::cout << c;
  }
  std::cout << std::endl;

  Crypto::HashingEnv hasher(Crypto::HashAlgorithm::Blake2);

  std::cout << "Plain Hash Result: ";
  for (const auto c : hasher.hashValue ) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::endl;
  std::cout << std::dec;

  hasher.plainData = crypto.plaintext;
  hasher.startHashing();

  std::cout << "Plain Hash Result: ";
  for (const auto c : hasher.hashValue ) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::endl;
  std::cout << std::dec;

  return QApplication::exec();
}
