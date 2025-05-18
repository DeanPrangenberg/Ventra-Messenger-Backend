#include "DirektChatScreen.h"

namespace Gui {

  DirektChatScreen::DirektChatScreen(QWidget *parent) : QWidget(parent) {
    chatScreenLayout = new QHBoxLayout(this);
    chatScreenLayout->setObjectName("directChatLayout");

    contactList = new ContactList(this);
    contactList->setObjectName("contactList");
    contactList->setMinimumWidth(250);
    contactList->setMaximumWidth(350);
    contactList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    for (int i = 0; i < 200; ++i) {
      contactList->addContact(
          QString("John Doe %1").arg(i),
          QPixmap(":/icons/res/icons/EmptyAccount.png")
      );
    }

    chatWindow = new ChatWindow(this);
    chatScreenLayout->addWidget(contactList);
    chatScreenLayout->addWidget(chatWindow, 1);  // Stretch = 1
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  DirektChatScreen::~DirektChatScreen() = default;

  void DirektChatScreen::loadChats(QList<chatData> &chatDataList) {
    for (const auto &chatData : chatDataList) {
      addChat(chatData);
    }
  }

  void DirektChatScreen::addChat(const chatData &chatData) {

  }

  void DirektChatScreen::removeChat(const QString &chatUUID) {

  }

  void DirektChatScreen::addMessagesToChat(const QString &chatUUID, const QList<MessageContainer> &messageContainers) {

  }

  QList<chatData> & DirektChatScreen::getAllChatData() {

  }

  QList<chatData> & DirektChatScreen::getChatData(const QString &chatUUID) {

  }

} // namespace Gui
