//
// Created by deanprangenberg on 25.05.25.
//

#include "DMChatGuiManager.h"

namespace Logic {
  DMChatGuiManager::DMChatGuiManager(Gui::DirektChatScreen *chatScreen) {
    if (!chatScreen) {
      throw std::invalid_argument("Chat screen cannot be null");
    }

    this->chatScreen = chatScreen;
  }

  QList<Gui::chatData> DMChatGuiManager::getAllChatData() const {
    QList<Gui::chatData> chatDataOutput;

    auto chatWindowMap = chatScreen->chatWindowMap;
    auto contactButtonMap = chatScreen->ButtonMap;

    for (const auto &chatWindowKey: chatWindowMap.keys()) {
      const auto &chatWindow = chatWindowMap.value(chatWindowKey);

      const auto &contactButton = contactButtonMap.value(chatWindowKey);

      auto chatData = Gui::chatData(
        chatWindow->getChatHistory(),
        contactButton->nameLabel->text(),
        contactButton->chatUUID,
        contactButton->avatarLabel->pixmap()
      );

      chatDataOutput.append(chatData);
    }

    return chatDataOutput;
  }

  Gui::chatData DMChatGuiManager::getSingleChatData(const QString &chatUUID) {
    auto chatWindowMap = chatScreen->chatWindowMap;
    auto contactButtonMap = chatScreen->ButtonMap;

    const auto &chatWindow = chatWindowMap.value(chatUUID);

    const auto &contactButton = contactButtonMap.value(chatUUID);

    auto chatDataOutput = Gui::chatData(
      chatWindow->getChatHistory(),
      contactButton->nameLabel->text(),
      contactButton->chatUUID,
      contactButton->avatarLabel->pixmap()
    );

    return chatDataOutput;
  }


  void DMChatGuiManager::addNewChat(const Gui::chatData &chatData) {
    chatScreen->contactList->addContact(chatData.name, chatData.chatUUID, chatData.avatar);
    auto *contactButton = chatScreen->contactList->getContactButtonPointer(chatData.chatUUID);
    if (!contactButton) {
      std::cerr << "Failed to create contact button for chat: " << chatData.chatUUID.toStdString() << std::endl;
      return;
    }

    auto *chatWindow = new Gui::ChatWindow(chatData.chatUUID, chatScreen);
    chatWindow->setObjectName("DMScreenChatWindow");
    chatWindow->setChatHistory(chatData.messageContainerList);

    chatScreen->ButtonMap.insert(chatData.chatUUID, contactButton);
    chatScreen->chatWindowMap.insert(chatData.chatUUID, chatWindow);

    chatScreen->chatWindowStack->addWidget(chatWindow);
    chatScreen->connect(contactButton, &QPushButton::clicked, chatScreen, [this, chatData]() {
      chatScreen->showChatbyID(chatData.chatUUID);
    });
  }

  void DMChatGuiManager::addNewChats(const QList<Gui::chatData> &datas) {
    for (const auto &data: datas) {
      addNewChat(data);
    }
  }

  void DMChatGuiManager::deleteChat(const QString &chatUUID) {
    if (auto *chatWindow = chatScreen->chatWindowMap.value(chatUUID)) {
      chatScreen->chatWindowStack->removeWidget(chatWindow);
      delete chatWindow;
    }
    if (auto *contactButton = chatScreen->ButtonMap.value(chatUUID)) {
      chatScreen->contactList->removeContact(chatUUID);
      delete contactButton;
    }
  }

  void DMChatGuiManager::removeAllChats() {
    for (const auto &chatUUID: chatScreen->chatWindowMap.keys()) {
      deleteChat(chatUUID);
    }
    chatScreen->chatWindowMap.clear();
    for (auto *button: chatScreen->ButtonMap.values()) {
      delete button;
    }
    chatScreen->ButtonMap.clear();
  }

  void DMChatGuiManager::updateChat(const QString &chatUUID, const QString &newName, const QPixmap &newAvatar) {
    auto contactButton = chatScreen->ButtonMap.value(chatUUID);
    contactButton->setText(newName);
    contactButton->avatarLabel->setPixmap(newAvatar);
  }

  void DMChatGuiManager::addNewMessages(QList<Gui::MessageContainer> message) {
    auto chatWindowMap = chatScreen->chatWindowMap;
    for (const auto &msg: message) {
      if (auto *chatWindow = chatWindowMap.value(msg.chatUUID)) {
        chatWindow->addNewMessages({msg});
      } else {
        std::cerr << "Chat window not found for UUID: " << msg.chatUUID.toStdString() << std::endl;
      }
    }
  }

  void DMChatGuiManager::deleteMessageFromChat(const QString &chatUUID, const QString &messageID) {
    auto chatWindow = chatScreen->chatWindowMap.value(chatUUID);
    for (auto it = chatWindow->messageList.begin(); it != chatWindow->messageList.end(); ++it) {
      if (it->messageUUID == messageID) {
        chatWindow->messageList.erase(it);
        break;
      }
    }
  }

  void DMChatGuiManager::updateMessageInChat(const QString &chatUUID, const Gui::MessageContainer &newContent) {
    auto chatWindow = chatScreen->chatWindowMap.value(chatUUID);
    for (auto it = chatWindow->messageList.begin(); it != chatWindow->messageList.end(); ++it) {
      if (it->messageUUID == newContent.messageUUID) {
        chatWindow->messageList.replace(std::distance(chatWindow->messageList.begin(), it), newContent);
        break;
      }
    }
  }

  void DMChatGuiManager::generateAndLoadTestChats(int numChats, int numMessagesPerChat) {
    QList<Gui::chatData> chatList;

    for (int chatIndex = 0; chatIndex < numChats; ++chatIndex) {
      QList<Gui::MessageContainer> messageList;
      const QString senderUUID = Crypto::GenerateID::uuid();

      for (int msgIndex = 0; msgIndex < numMessagesPerChat; ++msgIndex) {
        messageList.push_back(
          Gui::MessageContainer(
            senderUUID,
            Crypto::GenerateID::uuid(),
            QString("Message %1 in Chat %2").arg(msgIndex).arg(chatIndex),
            QString("18.05.2025, %1:%2").arg(12 + chatIndex).arg(msgIndex, 2, 10, QChar('0')),
            QString("Sender %1").arg(chatIndex),
            Crypto::GenerateID::uuid(),
            QPixmap(":/icons/res/icons/EmptyAccount.png"),
            msgIndex != 0
          )
        );
      }

      Gui::chatData chat{
        messageList,
        QString("ChatName_%1").arg(chatIndex),
        Crypto::GenerateID::uuid(),
        QPixmap(":/icons/res/icons/EmptyAccount.png")
      };

      chatList.push_back(chat);
    }

    addNewChats(chatList);
  }
} // Logic
