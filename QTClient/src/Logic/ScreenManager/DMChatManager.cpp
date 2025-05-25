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
    } else {
      std::cout << "DMChatManager initialized in production mode" << std::endl;
      std::cerr << "DMChatManager database password still is 'password123'" << std::endl;
      dbManager = std::make_unique<DMChatDBManager>("TEST_DMchatData.db", "password123", false);
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
      bool successChat = dbManager->insertChat(data.chatUUID, data.name, data.avatar);
      if (successChat) {
        std::cout << "Updated chat: " << data.chatUUID.toStdString() << std::endl;
      } else {
        std::cerr << "Failed to update chat: " << data.chatUUID.toStdString() << std::endl;
      }

      std::cout << "Inserting messages from chat in DB: " << data.chatUUID.toStdString() << std::endl;
      bool successMessages = dbManager->insertMessages(data.chatUUID, data.messageContainerList);
      if (successMessages) {
        std::cout << "Updated chat: " << data.chatUUID.toStdString() << std::endl;
      } else {
        std::cerr << "Failed to update chat: " << data.chatUUID.toStdString() << std::endl;
      }
    }
  }

  void DMChatManager::updateGuiFromDB() {

  }

  void DMChatManager::addNewChat(const Gui::chatData &data) {
  }

  void DMChatManager::addNewChats(const QList<Gui::chatData> &datas) {
  }

  void DMChatManager::deleteChat(const QString &chatUUID) {
  }

  void DMChatManager::updateChat(const QString &chatUUID, const QString &newName, const QPixmap &newAvatar) {
  }

  void DMChatManager::addNewMessages(QList<Gui::MessageContainer> message) {
  }

  void DMChatManager::deleteMessage(const QString &chatUUID, const QString &messageID) {
  }

  void DMChatManager::updateMessage(const QString &chatUUID, const Gui::MessageContainer &newContent) {
  }

  void DMChatManager::generateTestDBAndLoadToGui(int numChats, int numMessagesPerChat) {
  }
}
