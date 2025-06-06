//
// Created by deanprangenberg on 06.06.25.
//

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QWebSocket>
#include <QObject>
#include <iostream>

namespace Network {
  class WebSocketClient : public QObject {
  public:
    WebSocketClient(const QUrl &url, QObject *parent = nullptr);

  private:
    void onConnected();
    void testPacket();
    QWebSocket socket;
    QUrl serverUrl;
  };
} // Network

#endif //WEBSOCKET_H
