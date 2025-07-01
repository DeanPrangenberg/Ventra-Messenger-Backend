#include <QApplication>
#include <QPushButton>
#include <iostream>
#include "../test/WebSocketWorker.h"
#include <QThread>
#include "../../Shared/Crypto/Encryption/EncryptionEnv.h"
#include "../../Shared/Crypto/Hash/HashingEnv.h"
#include "ThreadPool/ThreadPool.h"
#include "../../Shared/Crypto/KeyEnv/KeyEnv.h"
#include "../../Shared/Crypto/DoubleRatchet/DoubleRatchet.h"
#include "HelperUtils/HelperUtils.h"
#include "Gui/MainWindow/MainWindow.h"
#include "../../Shared/Network/WebSocketClient.h"
#include "../../Shared/Network/Packages.h"
#include <QFile>

std::vector<std::vector<uint8_t> >
deriveSharedGroupSecret(const std::vector<Crypto::KeyEnv> &keys,
                        const std::vector<uint8_t> &salt,
                        const std::string &info,
                        size_t outSize = 32) {
  int const n = static_cast<int>(keys.size());
  std::vector<std::vector<uint8_t> > pubs(n);
  // Alle öffentlichen Keys einmal extrahieren
  for (int i = 0; i < n; ++i) {
    pubs[i] = keys[i].getPublicRaw();
  }

  std::vector<std::vector<uint8_t> > groupKeys(n);
  // Für jeden Teilnehmer i:
  for (int i = 0; i < n; ++i) {
    // 1) Starte mit Deinem eigenen Public:
    std::vector<uint8_t> acc = pubs[i];

    // 2) Führe n-1 DH-Operationen entlang der zyklischen Liste
    //    Teilnehmer (i+1)%n, (i+2)%n, ..., (i+n-1)%n
    for (int step = 1; step < n; ++step) {
      int partner = (i + step) % n;
      acc = keys[partner].deriveSharedSecret(acc);
    }
    // Nun ist acc = g^{x_i x_{i+1} ... x_{i-1}} = g^{x_0 x_1 ... x_{n-1}}

    // 3) Zieh mit HKDF Deinen symmetrischen Schlüssel
    std::vector<uint8_t> finalKey(outSize);
    Crypto::KDFEnv kdf(Crypto::KDFType::SHA3_512);
    kdf.startKDF(acc, salt, info, finalKey, outSize);

    groupKeys[i] = std::move(finalKey);

    // Optional: Debug-Ausgabe
    HelperUtils::printBytesAsHex("GroupKey[" + std::to_string(i) + "]", groupKeys[i]);
  }

  return groupKeys;
}

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
  for (const auto c: hasher.hashValue) {
    std::cout << std::hex << static_cast<int>(c);
  }
  std::cout << std::endl;
  std::cout << std::dec;
}

void test_double_ratchet() {
  if (DoubleRatchet::testOneSideDoubleRatchet() == false) {
    std::cout << "Double Ratchet On Side test failed!" << std::endl;
  } else {
    std::cout << "Double Ratchet On Side test succeeded!" << std::endl;
  }

  if (DoubleRatchet::testMixedDoubleRatchet() == false) {
    std::cout << "Double Ratchet mixed test failed!" << std::endl;
  } else {
    std::cout << "Double Ratchet mixed test succeeded!" << std::endl;
  }
}

void test_mulitBackendConnection(int numClients) {
  QList<QThread *> threads;
  for (int i = 0; i < numClients; ++i) {
    QThread *thread = new QThread;
    WebSocketWorker *worker = new WebSocketWorker(i);
    worker->moveToThread(thread);
    QObject::connect(thread, &QThread::started, worker, &WebSocketWorker::process);
    thread->start();
    threads.append(thread);
  }
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  Gui::MainWindow mainWindow;
  mainWindow.show();
  mainWindow.updateStyle(":/themes/themes/style.qss");

  test_mulitBackendConnection(1);

  return QApplication::exec();
}
