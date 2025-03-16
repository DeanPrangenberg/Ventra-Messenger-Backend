#include <QApplication>
  #include <QPushButton>
  #include <iostream>
  #include "Crypto/CryptoSet.h"

  int main(int argc, char *argv[]) {
      QApplication a(argc, argv);
      QPushButton button("Hello world!", nullptr);
      button.resize(200, 100);
      button.show();

      Crypto::CryptoSet crypto(Crypto::Algorithm::AES256);

      std::string plaintext = "Hello world!";
      crypto.plaintext.resize(plaintext.length());
      std::copy(plaintext.begin(), plaintext.end(), crypto.plaintext.begin());

      crypto.generateParameters();
      crypto.encrypt();

      std::cout << "Ciphertext: ";
      for (const auto c : crypto.ciphertext) {
          std::cout << std::hex << static_cast<int>(c);
      }
      std::cout << std::dec << std::endl;

      // Store ciphertext for decryption
      auto encrypted = crypto.ciphertext;
      crypto.ciphertext = encrypted;

      crypto.decrypt();

      std::cout << "Plaintext: ";
      for (const auto c : crypto.plaintext) {
          std::cout << c;
      }
      std::cout << std::endl;

      return QApplication::exec();
  }