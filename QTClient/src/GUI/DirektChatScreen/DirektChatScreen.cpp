#include "DirektChatScreen.h"
#include <iostream>

namespace Gui {
  DirektChatScreen::DirektChatScreen(QWidget *parent) : QWidget(parent) {
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
    chatScreenLayout->setObjectName("directChatLayout");
    chatScreenLayout->setContentsMargins(0, 0, 0, 0);

    // Initialize contact list
    contactList = new ContactList(this);
    contactList->setObjectName("contactList");
    contactList->setMinimumWidth(250);
    contactList->setMaximumWidth(350);
    contactList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Initialize chat window stack
    chatWindowStack = new QStackedWidget(this);
    chatWindowStack->setObjectName("chatWindowStack");

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
    // Create and add contact button
    contactList->addContact(data.name, data.chatUUID, data.avatar);
    auto *contactButton = contactList->getContactButtonPointer(data.chatUUID);
    if (!contactButton) {
      std::cerr << "Failed to create contact button for chat: " << data.chatUUID.toStdString() << std::endl;
      return;
    }

    // Create chat window
    auto *chatWindow = new ChatWindow(data.chatUUID, this);
    chatWindow->setChatHistory(data.messageContainerList);

    // Store references
    ButtonMap.insert(data.chatUUID, contactButton);
    ChatWindowMap.insert(data.chatUUID, chatWindow);

    // Add chat window to stack
    chatWindowStack->addWidget(chatWindow);

    // Connect button click
    connect(contactButton, &QPushButton::clicked, this, [this, chatUUID = data.chatUUID]() {
      showChatbyID(chatUUID);
    });
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

    if (auto *button = ButtonMap.take(chatUUID)) {
      // Button will be deleted by ContactList
      button->deleteLater();
    }
  }

  void DirektChatScreen::addMessagesToChat(const QString &chatUUID, const QList<MessageContainer> &messages) {
    if (auto *chatWindow = ChatWindowMap.value(chatUUID)) {
      chatWindow->addNewMessages(messages);
    }
  }

  void DirektChatScreen::generateAndLoadTestChats(int numChats, int numMessagesPerChat) {
    QList<chatData> chatList;

    for (int chatIndex = 0; chatIndex < numChats; ++chatIndex) {
      QList<MessageContainer> messageList;

      for (int msgIndex = 0; msgIndex < numMessagesPerChat; ++msgIndex) {
        messageList.push_back(
          MessageContainer(
            QString("UUID_SENDER_%1").arg(chatIndex),
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
        QString("UUID_CHAT_%1").arg(chatIndex),
        QPixmap(":/icons/res/icons/EmptyAccount.png")
      };

      chatList.push_back(chat);
    }

    // Neue Funktion nutzt vorhandene Methode
    loadChats(chatList);
  }
}
