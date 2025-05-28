//
// Created by deanprangenberg on 25.05.25.
//

#ifndef DMCHATGUIMANAGER_H
#define DMCHATGUIMANAGER_H

#include <QPixmap>
#include <QString>

#include "../../Gui/ChatWindow/Message.h"
#include "../../Gui/Gui_Structs_Enums.h"
#include "../../Gui/DirektChatScreen/DirektChatScreen.h"

namespace Gui {
  class DirektChatScreen;
}

namespace Logic {

class DMChatGuiManager {
friend class DMChatManager;
public:
  DMChatGuiManager(Gui::DirektChatScreen *chatScreen);

private:
  QList<Gui::chatData> getAllChatData() const;
  Gui::chatData getSingleChatData(const QString &chatUUID);

  void addNewChat(const Gui::chatData &data);
  void addNewChats(const QList<Gui::chatData> &datas);
  void deleteChat(const QString &chatUUID);
  void removeAllChats();
  void updateChat(const QString &chatUUID, const QString &newName, const QPixmap &newAvatar);

  void addNewMessages(QList<Gui::MessageContainer> message);
  void deleteMessageFromChat(const QString &chatUUID, const QString &messageID);
  void updateMessageInChat(const QString &chatUUID, const Gui::MessageContainer &newContent);

  void generateAndLoadTestChats(int numChats, int numMessagesPerChat);
  Gui::DirektChatScreen *chatScreen;
};

} // Logic

#endif //DMCHATGUIMANAGER_H
