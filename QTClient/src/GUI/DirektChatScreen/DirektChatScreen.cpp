#include "DirektChatScreen.h"
#include <iostream>

namespace Gui {
  DirektChatScreen::DirektChatScreen(QWidget *parent)
    : QWidget(parent), db("DMchatData_debug.db", "password123", true) {
    initializeLayout();
  }

  DirektChatScreen::~DirektChatScreen() {
    // Clean up maps
    qDeleteAll(ChatWindowMap);
    ChatWindowMap.clear();
    ButtonMap.clear();

    delete chatScreenLayout;
    delete contactList;
    delete chatWindowStack;
  }

  void DirektChatScreen::initializeLayout() {
    // Setup main layout
    chatScreenLayout = new QHBoxLayout(this);
    chatScreenLayout->setObjectName("DMScreenLayout");
    chatScreenLayout->setContentsMargins(0, 0, 0, 0);

    // Initialize contact list
    contactList = new ContactList(this);
    contactList->setObjectName("DMScreenContactList");
    contactList->setObjectName("contactList");
    contactList->setMinimumWidth(250);
    contactList->setMaximumWidth(350);
    contactList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Initialize chat window stack
    chatWindowStack = new QStackedWidget(this);
    chatWindowStack->setObjectName("DMScreenChatWindowStack");

    // Add widgets to layout
    chatScreenLayout->addWidget(contactList);
    chatScreenLayout->addWidget(chatWindowStack, 1);

    setLayout(chatScreenLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  void DirektChatScreen::loadChats(QList<chatData> &chatDataList) {
    for (const auto &data: chatDataList) {
      addChat(data);
    }
  }

  void DirektChatScreen::addChat(const chatData &data) {
    // Create UI components in main thread
    contactList->addContact(data.name, data.chatUUID, data.avatar);
    auto *contactButton = contactList->getContactButtonPointer(data.chatUUID);
    if (!contactButton) {
      std::cerr << "Failed to create contact button for chat: " << data.chatUUID.toStdString() << std::endl;
      return;
    }

    auto *chatWindow = new ChatWindow(data.chatUUID, this);
    chatWindow->setObjectName("DMScreenChatWindow");
    chatWindow->setChatHistory(data.messageContainerList);

    ButtonMap.insert(data.chatUUID, contactButton);
    ChatWindowMap.insert(data.chatUUID, chatWindow);

    // Save to DB in background with batch processing
    pendingSaves++;
    updateDebugInfo();

    Utils::ThreadPool::getInstance().addTask([this, data]() {
      QMutexLocker locker(&dbMutex);

      // Insert chat first
      bool chatSuccess = db.insertChat(data.chatUUID, data.name, data.avatar);
      std::cout << "Saving chat " << data.chatUUID.toStdString()
          << " - " << (chatSuccess ? "Success" : "Failed") << std::endl;

      // Batch insert all messages
      if (chatSuccess && !data.messageContainerList.isEmpty()) {
        bool msgsSuccess = db.insertMessages(data.chatUUID, data.messageContainerList);
        std::cout << "Batch saving " << data.messageContainerList.size()
            << " messages in chat " << data.chatUUID.toStdString()
            << " - " << (msgsSuccess ? "Success" : "Failed") << std::endl;
      }

      pendingSaves--;
      updateDebugInfo();
    });

    chatWindowStack->addWidget(chatWindow);
    connect(contactButton, &QPushButton::clicked, this, [this, chatUUID = data.chatUUID]() {
      showChatbyID(chatUUID);
    });
  }

  void DirektChatScreen::addMessagesToChat(const QString &chatUUID, const QList<MessageContainer> &messages) {
    if (auto *chatWindow = ChatWindowMap.value(chatUUID)) {
      chatWindow->addNewMessages(messages);

      // Save messages in batch
      pendingSaves++;
      updateDebugInfo();

      Utils::ThreadPool::getInstance().addTask([this, chatUUID, messages]() {
        QMutexLocker locker(&dbMutex);
        bool success = db.insertMessages(chatUUID, messages);
        std::cout << "Batch saving " << messages.size() << " messages in chat "
            << chatUUID.toStdString() << " - "
            << (success ? "Success" : "Failed") << std::endl;

        pendingSaves--;
        updateDebugInfo();
      });
    }
  }

  void DirektChatScreen::showChatbyID(const QString &chatUUID) {
    auto *chatWindow = ChatWindowMap.value(chatUUID);
    if (!chatWindow) {
      std::cerr << "Chat window not found for UUID: " << chatUUID.toStdString() << std::endl;
      return;
    }

    // Uncheck all buttons and check the selected one
    for (auto *button: ButtonMap.values()) {
      button->setChecked(button->chatUUID == chatUUID);
    }

    chatWindowStack->setCurrentWidget(chatWindow);
  }

  void DirektChatScreen::removeChat(const QString &chatUUID) {
    if (auto *chatWindow = ChatWindowMap.take(chatUUID)) {
      chatWindowStack->removeWidget(chatWindow);
      delete chatWindow;
    }
  }

  void DirektChatScreen::generateAndLoadTestChats(int numChats, int numMessagesPerChat) {
    QList<chatData> chatList;

    for (int chatIndex = 0; chatIndex < numChats; ++chatIndex) {
      QList<MessageContainer> messageList;
      const QString senderUUID = Crypto::GenerateID::uuid();

      for (int msgIndex = 0; msgIndex < numMessagesPerChat; ++msgIndex) {
        messageList.push_back(
          MessageContainer(
            senderUUID,
            QString("Message %1 in Chat %2").arg(msgIndex).arg(chatIndex),
            QString("18.05.2025, %1:%2").arg(12 + chatIndex).arg(msgIndex, 2, 10, QChar('0')),
            QString("Sender %1").arg(chatIndex),
            QPixmap(":/icons/res/icons/EmptyAccount.png"),
            msgIndex != 0
          )
        );
      }

      chatData chat{
        messageList,
        QString("ChatName_%1").arg(chatIndex),
        Crypto::GenerateID::uuid(),
        QPixmap(":/icons/res/icons/EmptyAccount.png")
      };

      chatList.push_back(chat);
    }

    // Neue Funktion nutzt vorhandene Methode
    loadChats(chatList);
  }

  void DirektChatScreen::loadChatsFromDB() {
    if (isLoading.exchange(true)) return;
    std::cout << "Starting to load chats from database..." << std::endl;
    updateDebugInfo();

    auto &pool = Utils::ThreadPool::getInstance();

    // Load all chats first
    auto future = pool.addTask([this]() {
      QMutexLocker locker(&dbMutex);
      auto chats = db.getAllChats();
      std::cout << "Found " << chats.size() << " chats in database" << std::endl;
      return chats;
    });

    auto chats = future.get();
    std::vector<std::future<chatData> > chatFutures;

    // Load messages for each chat
    for (const auto &chatInfo: chats) {
      std::cout << chatInfo.uuid.toStdString() << " - " << chatInfo.name.toStdString() << std::endl;
      std::cout << "Loading messages for chat: " << chatInfo.uuid.toStdString() << std::endl;
      chatFutures.push_back(pool.addTask([this, chatInfo]() -> chatData {
        QMutexLocker locker(&dbMutex);
        auto messages = db.getChatMessages(chatInfo.uuid);
        std::cout << "Loaded " << messages.size() << " messages for chat "
            << chatInfo.uuid.toStdString() << std::endl;

        QList<MessageContainer> messageContainers;
        for (const auto &msgInfo: messages) {
          QPixmap avatar;
          if (!msgInfo.senderAvatar.isEmpty()) {
            avatar.loadFromData(msgInfo.senderAvatar);
          }
          if (avatar.isNull()) {
            avatar = QPixmap(":/icons/res/icons/EmptyAccount.png");
          }

          messageContainers.append(MessageContainer(
            msgInfo.senderUuid,
            msgInfo.content,
            msgInfo.timestamp,
            msgInfo.senderName,
            avatar,
            msgInfo.isHistory
          ));
        }

        QPixmap chatAvatar;
        if (!chatInfo.avatar.isEmpty()) {
          chatAvatar.loadFromData(chatInfo.avatar);
        }
        if (chatAvatar.isNull()) {
          chatAvatar = QPixmap(":/icons/res/icons/ChatDark.png");
        }

        return chatData{messageContainers, chatInfo.name, chatInfo.uuid, chatAvatar};
      }));
    }

    // Process loaded chats
    for (auto &future: chatFutures) {
      auto data = future.get();
      ChatWindow *chatWindow = ChatWindowMap.value(data.chatUUID);

      if (chatWindow) {
        // Update existing chat window
        chatWindow->addNewMessages(data.messageContainerList);
      } else {
        // Create new chat window and UI elements
        contactList->addContact(data.name, data.chatUUID, data.avatar);
        chatWindow = new ChatWindow(data.chatUUID, this);
        chatWindow->setObjectName("DMScreenChatWindow");
        chatWindow->setChatHistory(data.messageContainerList);

        auto *contactButton = contactList->getContactButtonPointer(data.chatUUID);
        if (contactButton) {
          ButtonMap.insert(data.chatUUID, contactButton);
          ChatWindowMap.insert(data.chatUUID, chatWindow);
          chatWindowStack->addWidget(chatWindow);

          connect(contactButton, &QPushButton::clicked, this, [this, chatUUID = data.chatUUID]() {
            showChatbyID(chatUUID);
          });
        }
      }
    }

    isLoading = false;
    std::cout << "Finished loading all chats from database" << std::endl;
    updateDebugInfo();
  }

  void DirektChatScreen::updateDebugInfo() {
    std::cout << "DB Status: "
        << "Pending saves: " << pendingSaves
        << " | Chats loaded: " << ChatWindowMap.size()
        << " | Loading: " << (isLoading ? "Yes" : "No")
        << std::endl;
  }
}
