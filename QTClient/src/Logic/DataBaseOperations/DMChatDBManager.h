//
// Created by deanprangenberg on 25.05.25.
//

#ifndef DMCHATDBMANAGER_H
#define DMCHATDBMANAGER_H

#include <qiodevice.h>
#include <QString>
#include <QBuffer>
#include "../../Gui/Gui_Structs_Enums.h"
#include "../../Database/LocalDatabase.h"

namespace Logic {
  class DMChatManager;

  class DMChatDBManager : public LocalDatabase {
    friend class DMChatManager;

  private:
    struct ChatInfo {
      QString uuid;
      QString name;
      QByteArray avatar;
    };

    DMChatDBManager(const fs::path &dbPath, const std::string &password, bool debugMode);

    QList<ChatInfo> getAllChats();

    QList<Gui::MessageContainer> getChatMessages(const QString &chatUuid);

    bool insertChat(const QString &chatUUID, const QString &name, const QPixmap &avatar);

    bool insertMessage(const QString &chatUUID, const Gui::MessageContainer &msg);

    bool insertMessages(const QString &chatUUID, const QList<Gui::MessageContainer> &messages);


    bool createChatTables();
  };
} // Logic

#endif //DMCHATDBMANAGER_H
