//
// Created by deanprangenberg on 06.06.25.
//

#include <iostream>
#include <QCoreApplication>
#include <thread>
#include "../../../Shared/Network/WebSocketServer.h"

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  Network::WebSocketServer server(8881); // Port definieren

  return a.exec();
}
