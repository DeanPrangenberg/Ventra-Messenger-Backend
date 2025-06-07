//
// Created by deanprangenberg on 07.06.25.
//

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QList>

namespace Network {

  class WebSocketServer : public QObject {
    Q_OBJECT

  public:
    explicit WebSocketServer(quint16 port, QObject *parent = nullptr);
    ~WebSocketServer() override;

  private slots:
    void onNewConnection();
    void processTextMessage(const QString &message);
    void socketDisconnected();

  private:
    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
  };

} // Network

#endif //WEBSOCKETSERVER_H
