//
// Created by deanprangenberg on 25.05.25.
//

#include "DMChatManager.h"

namespace Logic {
  DMChatManager::DMChatManager(Gui::DirektChatScreen *chatScreen, bool testMode) {
    if (!chatScreen) {
      throw std::runtime_error("Chat screen pointer is null");
    }

    this->chatScreen = chatScreen;

    if (testMode) {
      std::cout << "DMChatManager initialized in test mode" << std::endl;
      dbManager = std::make_unique<DMChatDBManager>("TEST_DMchatData.db", "password123", true);
      bool testdata = generateTestDBAndLoadToGui(10, 100);

      if (!testdata) {
        throw std::runtime_error("DMChatManager could not be initialized in test mode");
      }
    } else {
      std::cout << "DMChatManager initialized in production mode" << std::endl;
      std::cerr << "DMChatManager database password still is 'password123'" << std::endl;
      dbManager = std::make_unique<DMChatDBManager>("Enc_DMchatData.db", "password123", false);
    }

    guiManager = std::make_unique<DMChatGuiManager>(chatScreen);
  }

  void DMChatManager::updateDBfromGui() {
    auto guiChatData = guiManager->getAllChatData();
    if (guiChatData.isEmpty()) {
      std::cout << "No chat data to update in database" << std::endl;
      return;
    }
    std::cout << "Updating database with " << guiChatData.size() << " chat(s) from GUI" << std::endl;

    for (const auto &data: guiChatData) {
      if (data.chatUUID.isEmpty()) {
        std::cerr << "Skipping chat with empty UUID" << std::endl;
        continue;
      }

      std::cout << "Inserting chat in DB: " << data.chatUUID.toStdString() << std::endl;
      bool successChat = dbManager->insertChat(data);
      if (successChat) {
        std::cout << "Updated chat: " << data.chatUUID.toStdString() << std::endl;
      } else {
        std::cerr << "Failed to update chat: " << data.chatUUID.toStdString() << std::endl;
      }
    }
  }

  void DMChatManager::updateGuiFromDB() {
    auto chatList = dbManager->getAllChats();

    guiManager->removeAllChats();

    for (const auto &chat: chatList) {
      if (chat.chatUUID.isEmpty()) {
        std::cerr << "Skipping chat with empty UUID" << std::endl;
        continue;
      }

      std::cout << "Adding chat to GUI: " << chat.chatUUID.toStdString() << std::endl;
      guiManager->addNewChat(chat);
    }
  }

  void DMChatManager::addNewChat(const Gui::chatData &data) {
    dbManager->insertChat(data);
    guiManager->addNewChat(data);
  }

  void DMChatManager::addNewChats(const QList<Gui::chatData> &datas) {
    dbManager->insertChats(datas);
    guiManager->addNewChats(datas);
  }

  void DMChatManager::deleteChat(const QString &chatUUID) {
    dbManager->deleteChat(chatUUID);
    guiManager->deleteChat(chatUUID);
  }

  void DMChatManager::updateChat(const QString &chatUUID, const QString &newName, const QPixmap &newAvatar) {
    auto chatMessages = dbManager->getChatMessages(chatUUID);

    auto newChatData = Gui::chatData(
      chatMessages,
      newName,
       chatUUID,
      newAvatar
      );

    dbManager->updateChat(newChatData);
    guiManager->updateChat(chatUUID, newName, newAvatar);
  }

  void DMChatManager::addNewMessages(QList<Gui::MessageContainer> message) {
    dbManager->insertMessages(message);
    guiManager->addNewMessages(message);
  }

  void DMChatManager::deleteMessage(const QString &chatUUID, const QString &messageID) {
    dbManager->deleteMessage(messageID);
    guiManager->deleteMessageFromChat(chatUUID, messageID);
  }

  void DMChatManager::updateMessage(const QString &chatUUID, const Gui::MessageContainer &newContent) {
    dbManager->updateMessage(newContent);
    guiManager->updateMessageInChat(chatUUID, newContent);
  }

  bool DMChatManager::generateTestDBAndLoadToGui(int numChats, int numMessagesPerChat) {
    if (dbManager->getAllChats().size() > 0) {
      std::cout << "DMChat Database already has data, skipping test data generation." << std::endl;
      return true; // Database already has data, no need to generate test data
    }

    QList<Gui::chatData> chats;

    for (int i = 0; i < numChats; i++) {
      Gui::chatData chat;
      QList<Gui::MessageContainer> chatMessages;

      chat.chatUUID = Crypto::GenerateID::uuid();
      chat.name = "Test Chat " + QString::number(i + 1);
      chat.avatar = QPixmap(":/icons/res/icons/EmptyAccount.png");
      if (chat.avatar.isNull()) {
        qWarning() << "Avatar image not found or failed to load!";
      }

      for (int j = 0; j < numMessagesPerChat; j++) {
        Gui::MessageContainer msgs;
        msgs.messageUUID = Crypto::GenerateID::uuid();
        msgs.chatUUID = chat.chatUUID;
        msgs.avatar = chat.avatar;
        msgs.senderUUID = Crypto::GenerateID::uuid();
        msgs.senderName = "Test User " + QString::number(j + 1);
        msgs.message = "This is a test message " + QString::number(j + 1) + " in chat " + chat.name;
        msgs.time = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
        msgs.isFollowUp = (j != 0);
        chatMessages.append(msgs);
      }

      chat.messageContainerList = chatMessages;
      chats.append(chat);
    }

    return dbManager->insertChats(chats);
  }
}
