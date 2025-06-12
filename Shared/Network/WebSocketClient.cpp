//
// Created by deanprangenberg on 07.06.25.
//

#include "WebSocketClient.h"

namespace Network {
  WebSocketClient::WebSocketClient(const QUrl &url, QObject *parent)
    : QObject(parent), serverUrl(url) {
    connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&socket, &QWebSocket::pong, this, [](quint64 elapsedTime, const QByteArray &payload) {
    std::cout << "Ping answer after " << std::to_string(elapsedTime) << "ms – Payload: " << payload.toStdString();
});
    socket.open(serverUrl);
  }

  void WebSocketClient::onConnected() {
    std::cout << "Connected to server" << std::endl;
    testPacket();
  }

  void WebSocketClient::testPacket() {
    for (int i = 0; i < 10; ++i) {
      std::cout << "Sending test packet " << i + 1 << std::endl;
      socket.sendTextMessage("Test message " + QString::number(i + 1));
      sleep(1);
    }
  }


} // Network