//
// Created by deanprangenberg on 22.05.25.
//

#ifndef CHATDB_H
#define CHATDB_H

#include <qiodevice.h>
#include <QString>
#include <QBuffer>
#include "../GUI/ChatWindow/Message.h"

#include "LocalDatabase.h"

class DmChatDB : public LocalDatabase {
public:
  struct ChatInfo {
    QString uuid;
    QString name;
    QByteArray avatar;
  };

  struct MessageInfo {
    QString senderUuid;
    QString content;
    QString timestamp;
    QString senderName;
    QByteArray senderAvatar;
    bool isHistory;
  };

  QList<ChatInfo> getAllChats();

  QList<MessageInfo> getChatMessages(const QString &chatUuid);

  DmChatDB(const fs::path &dbPath, const std::string &password, bool debugMode);

  bool insertChat(const QString& chatUUID, const QString& name, const QPixmap& avatar);

  bool insertMessage(const QString &chatUUID, const Gui::MessageContainer &msg);

  bool insertMessages(const QString &chatUUID, const QList<Gui::MessageContainer> &messages);

private:

  bool createChatTables();
};


#endif //CHATDB_H
