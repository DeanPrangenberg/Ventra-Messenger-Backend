//
// Created by deanprangenberg on 25.05.25.
//

#ifndef DMCHATMANAGER_H
#define DMCHATMANAGER_H

#include <QDateTime>

#include "../../Gui/DirektChatScreen/DirektChatScreen.h"
#include "../DataBaseOperations/DMChatDBManager.h"
#include "../GuiUpdates/DMChatGuiManager.h"

namespace Logic {
  class DMChatManager {
  public:
    DMChatManager(Gui::DirektChatScreen *chatScreen, bool testMode);

    void updateDBfromGui();
    void updateGuiFromDB();

    void addNewChat(const Gui::chatData &data);
    void addNewChats(const QList<Gui::chatData> &datas);
    void deleteChat(const QString &chatUUID);
    void updateChat(const QString &chatUUID, const QString &newName, const QPixmap &newAvatar);

    void addNewMessages(QList<Gui::MessageContainer> message);
    void deleteMessage(const QString &chatUUID, const QString &messageID);
    void updateMessage(const QString &chatUUID, const Gui::MessageContainer &newContent);

    bool generateTestDBAndLoadToGui(int numChats, int numMessagesPerChat);

  private:
    std::unique_ptr<DMChatDBManager> dbManager;
    Gui::DirektChatScreen *chatScreen;
    std::unique_ptr<DMChatGuiManager> guiManager;
  };
}

#endif //DMCHATMANAGER_H
