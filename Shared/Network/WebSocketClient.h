//
// Created by deanprangenberg on 07.06.25.
//

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QWebSocket>
#include <QObject>
#include <memory>
#include "../Crypto/Encryption/EncryptionEnv.h"
#include "../Crypto/Hash/HashingEnv.h"
#include "../Crypto/KeyEnv/KeyEnv.h"
#include "../Converter/HexConverter.h"
#include <iostream>
#include "Packages.h"

namespace Network {
  class WebSocketClient : public QObject {
  public:
    WebSocketClient(const QUrl &url, QObject *parent = nullptr);

    void testPacket();

  private:
    void sendHandshakeData();

    std::unique_ptr<Crypto::KeyEnv> keyPairEnv;
    std::unique_ptr<Crypto::KeyEnv> ivEnv;
    std::unique_ptr<Crypto::EncryptionEnv> encEnv;
    std::unique_ptr<Crypto::HashingEnv> hashingEnv;
    std::vector<uint8_t> sharedSecret;

    void onConnected();

    QWebSocket socket;
    QUrl serverUrl;
    bool handshakeDone;
  };
} // Network

#endif //WEBSOCKETCLIENT_H
