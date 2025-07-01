//
// Created by deanprangenberg on 17.06.25.
//

#include "Packages.h"
#include <QDateTime>

QJsonObject Packages::makeMessagePkg(const QString &content,
                                     const QString &timestamp, const QString &senderID,
                                     const QString &messageType, const QString &receiverID,
                                     const QString &messageID) {
  QJsonObject data;
  data["content"] = content;
  data["timestamp"] = timestamp;
  data["senderID"] = senderID;
  data["messageType"] = messageType;
  data["receiverID"] = receiverID;
  data["messageID"] = messageID;
  return data;
}

QString Packages::convertPkgToJsonStr(const QJsonObject &pkg) {
  QJsonDocument doc(pkg);
  return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QJsonObject Packages::testMessage() {
  return makeMessagePkg(
    "TEST_ Hello, this is a test message! _TEST",
    QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss:zzz"),
    "TEST_ user123 _TEST",
    "DM",
    "TEST_ user456 _TEST",
    "TEST_ msg789 _TEST"
  );
}