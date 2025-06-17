//
// Created by deanprangenberg on 17.06.25.
//

#ifndef PACKAGES_H
#define PACKAGES_H

#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

class Packages {
public:
  QJsonObject makePkg(const QString& pkgType, const QJsonObject& data);
  QJsonObject makeMessagePkg(const QString& content,
                      const QString& timestamp, const QString& senderID,
                      const QString& messageType, const QString& receiverID,
                      const QString& messageID);
  static QString convertPkgToJsonStr(const QJsonObject& pkg);
  QJsonObject testMessage();
};

#endif //PACKAGES_H
