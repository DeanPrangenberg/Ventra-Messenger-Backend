//
// Created by deanprangenberg on 17.05.25.
//

#include "ChatInputBar.h"

namespace Gui {
  ChatInputBar::ChatInputBar(QWidget *parent) : QWidget(parent) {
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    chatLineEdit = new QLineEdit(this);

    sendButton = new QPushButton("Send", this);
    sendButton->setObjectName("sendButton");

    attachButton = new QPushButton("Attach", this);
    attachButton->setObjectName("attachButton");

    audioButton = new QPushButton("Audio", this);
    audioButton->setObjectName("audioButton");

    layout->addWidget(attachButton);
    layout->addWidget(chatLineEdit);
    layout->addWidget(sendButton);
    layout->addWidget(audioButton);
  }

  ChatInputBar::~ChatInputBar() {
    delete chatLineEdit;
    delete sendButton;
    delete attachButton;
    delete audioButton;
    delete layout;
  }
} // Gui
