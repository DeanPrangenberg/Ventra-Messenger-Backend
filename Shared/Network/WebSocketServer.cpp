//
// Created by deanprangenberg on 07.06.25.
//

#include "WebSocketServer.h"
#include <iostream>

namespace Network {
  WebSocketServer::WebSocketServer(quint16 port, QObject *parent) : QObject(parent),
                                                                    m_server(new QWebSocketServer(
                                                                      QStringLiteral("Test Server"),
                                                                      QWebSocketServer::NonSecureMode, this)) {
    if (m_server->listen(QHostAddress::Any, port)) {
      std::cout << "WebSocket Server started on port" << port << std::endl;
      connect(m_server, &QWebSocketServer::newConnection,
              this, &WebSocketServer::onNewConnection);
    } else {
      std::cout << "WebSocket Server could not start!" << std::endl;
    }
  }

  WebSocketServer::~WebSocketServer() {
    m_server->close();
    qDeleteAll(m_clients);
  }

  void WebSocketServer::onNewConnection() {
    QWebSocket *client = m_server->nextPendingConnection();

    connect(client, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
    connect(client, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);

    m_clients << client;
    std::cout << "New client connected" << std::endl;
  }

  void WebSocketServer::processTextMessage(const QString &message) {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    std::cout << "Message received:" << message.toStdString() << std::endl;

    // Echo back
    if (client) {
      client->sendTextMessage(QString("Echo: ") + message);
    }
  }

  void WebSocketServer::socketDisconnected() {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
      m_clients.removeAll(client);
      client->deleteLater();
      std::cout << "Client disconnected" << std::endl;
    }
  }
} // Network
