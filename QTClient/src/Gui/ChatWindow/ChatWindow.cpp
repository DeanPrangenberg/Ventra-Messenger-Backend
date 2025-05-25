//
// Created by deanprangenberg on 17.05.25.
//

#include "ChatWindow.h"

namespace Gui {
  ChatWindow::ChatWindow(const QString chatUUIDIn, QWidget *parent) : QWidget(parent) {
    messageContainer = new QWidget(this);
    messageContainer->setObjectName("ChatWindowContainer");
    messageContainerLayout = new QVBoxLayout(messageContainer);

    WindowLayout = new QVBoxLayout(this);
    WindowLayout->setContentsMargins(0, 0, 0, 0);

    chatInputBar = new ChatInputBar(this);
    chatInputBar->setObjectName("ChatWindowChatInputBar");
    chatInputBar->setMaximumHeight(50);

    chatArea = new QScrollArea(this);
    chatArea->setObjectName("ChatWindowScrollChatArea");
    chatArea->setWidget(messageContainer);
    chatArea->setWidgetResizable(true);
    chatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    chatUUID = chatUUIDIn;

    // Add chatArea to WindowLayout first, then chatInputBar
    WindowLayout->addWidget(chatArea);
    WindowLayout->addWidget(chatInputBar);

    setContentsMargins(0, 0, 0, 0);
    setLayout(WindowLayout);
  }

  ChatWindow::~ChatWindow() {
    messageList.clear();
    delete messageContainer;
    delete messageContainerLayout;
    delete WindowLayout;
    delete chatArea;
    delete chatInputBar;
  }

  void ChatWindow::setChatHistory(const QList<MessageContainer> &messageListIn) {
    messageList = messageListIn;  // Now copying the list
    updateDisplayedMessage();     // Update the display after setting
  }

  QList<MessageContainer> &ChatWindow::getChatHistory() {
    return messageList;
  }

  void ChatWindow::addNewMessages(QList<MessageContainer> messageContainers) {
    messageList.append(messageContainers);
    updateDisplayedMessage();
  }

  void ChatWindow::addOldMessages(QList<MessageContainer> messageContainers) {
    messageList.append(messageContainers);
    updateDisplayedMessage();
  }

  void ChatWindow::updateDisplayedMessage() {
    GuiHelper::clearLayout(messageContainerLayout);
    for (const auto &messageContent: messageList) {
      Message *msg = new Message(messageContent, messageContainer);
      msg->setObjectName("ChatWindowMessage");
      messageContainerLayout->addWidget(msg);
    }
    messageContainerLayout->addStretch(1);
  }
} // Gui
