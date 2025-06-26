//
// Created by deanprangenberg on 26.06.25.
//

#ifndef WEBSOCKETWORKER_H
#define WEBSOCKETWORKER_H

#include <QObject>
#include "../../Shared/Network/WebSocketClient.h"

class WebSocketWorker : public QObject {
  Q_OBJECT
public:
  WebSocketWorker(int id) : id(id) {}
public slots:
    void process() {
    Network::WebSocketClient client(QUrl("ws://127.0.0.1:8881/ws"));
    std::cout << "WebSocket client " << id << " started" << std::endl;
    QEventLoop loop;
    loop.exec();
  }
private:
  int id;
};

#endif //WEBSOCKETWORKER_H
