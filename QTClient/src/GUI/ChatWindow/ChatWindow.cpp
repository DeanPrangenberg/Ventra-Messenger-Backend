//
// Created by deanprangenberg on 17.05.25.
//

#include "ChatWindow.h"

namespace Gui {
  ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    messageContainer = new QWidget(this);
    messageContainerLayout = new QVBoxLayout(messageContainer);

    WindowLayout = new QVBoxLayout(this);
    WindowLayout->setContentsMargins(0, 0, 0, 0);

    chatInputBar = new ChatInputBar(this);
    chatInputBar->setMaximumHeight(50);

    chatArea = new QScrollArea(this);
    chatArea->setWidget(messageContainer);
    chatArea->setWidgetResizable(true);
    chatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Add messages to messageContainerLayout
    for (int i = 0; i < 12; ++i) {
      MessageContainer messageContent;
      messageContent.message =
          "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      messageContent.time = "18.05.2025, 12:00";
      messageContent.senderName = "John Doe";
      messageContent.avatar = QPixmap(":/icons/res/icons/EmptyAccount.png");
      Message *msg;
      if (i % 2 == 1) {
        messageContent.isFollowUp = true;
        msg = new Message(messageContent, messageContainer);
      } else {
        messageContent.isFollowUp = false;
        msg = new Message(messageContent, messageContainer);
      }
      messageContainerLayout->addWidget(msg);
      messageList.push_back(messageContent);
    }
    messageContainerLayout->addStretch(1);

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

  void ChatWindow::setChatHistory(QList<MessageContainer> &messageListIn) {
    messageList = messageListIn;
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
    GUIHelper::clearLayout(messageContainerLayout);
    for (const auto &messageContent: messageList) {
      Message *msg = new Message(messageContent, messageContainer);
      messageContainerLayout->addWidget(msg);
    }
    messageContainerLayout->addStretch(1);
  }
} // Gui
