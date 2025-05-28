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
  public:
    DMChatDBManager(const fs::path &dbPath, const std::string &password, bool debugMode);
  private:

    QList<Gui::chatData> getAllChats();

    QList<Gui::MessageContainer> getChatMessages(const QString &chatUuid);

    bool insertChat(const Gui::chatData &chat);

    bool insertChats(const QList<Gui::chatData> &chats);

    bool insertMessage(const Gui::MessageContainer &msg);

    bool insertMessages(const QList<Gui::MessageContainer> &messages);

    bool deleteChat(const QString &chatUUID);

    bool deleteChats(const QList<QString> &chatUUIDs);

    bool deleteMessage(const QString &messageUUID);

    bool deleteMessages(const QList<QString> &messageUUIDs);

    bool updateChat(const Gui::chatData &chat);

    bool updateChats(const QList<Gui::chatData> &chats);

    bool updateMessage(const Gui::MessageContainer &msg);

    bool updateMessages(const QList<Gui::MessageContainer> &messages);


    bool createChatTables();
  };
} // Logic

#endif //DMCHATDBMANAGER_H
