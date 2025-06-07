//
// Created by deanprangenberg on 07.06.25.
//

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QWebSocket>
#include <QObject>
#include <iostream>

namespace Network {
  class WebSocketClient : public QObject {
  public:
    WebSocketClient(const QUrl &url, QObject *parent = nullptr);
    void testPacket();

  private:
    void onConnected();
    QWebSocket socket;
    QUrl serverUrl;
  };
} // Network

#endif //WEBSOCKETCLIENT_H
