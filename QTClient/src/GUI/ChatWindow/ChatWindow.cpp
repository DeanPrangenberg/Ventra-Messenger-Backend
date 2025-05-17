//
// Created by deanprangenberg on 17.05.25.
//

#include "ChatWindow.h"

namespace Gui {
  ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    messageContainer = new QWidget(this);
    messageContainerLayout = new QVBoxLayout(messageContainer);
    WindowLayout = new QVBoxLayout(this);
    chatInputBar = new ChatInputBar(this);
    chatInputBar->setMaximumHeight(50);

    chatArea = new QScrollArea(this);
    chatArea->setWidget(messageContainer);
    chatArea->setWidgetResizable(true);
    chatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Add messages to messageContainerLayout
    for (int i = 0; i < 100; ++i) {
      MessageContainer messageContent;
      messageContent.message =
          "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
      messageContent.time = "12:00";
      messageContent.senderName = "John Doe";
      messageContent.avatar = QPixmap(":/icons/res/icons/EmptyAccount.png");
      auto *msg = new Message(messageContent, this);
      inputBars.push_back(msg);
      messageContainerLayout->addWidget(msg);
    }

    // Add chatArea to WindowLayout first, then chatInputBar
    WindowLayout->addWidget(chatArea);
    WindowLayout->addWidget(chatInputBar);

    setContentsMargins(0, 0, 0, 0);
    setLayout(WindowLayout);
  }

  ChatWindow::~ChatWindow() {
    for (Gui::Message *msg: inputBars) {
      delete msg;
    }
    inputBars.clear();
    delete messageContainer;
    delete messageContainerLayout;
    delete WindowLayout;
    delete chatArea;
    delete chatInputBar;
  }
} // Gui
