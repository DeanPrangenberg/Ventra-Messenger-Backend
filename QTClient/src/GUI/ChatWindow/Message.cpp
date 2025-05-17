//
// Created by deanprangenberg on 17.05.25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Message.h" resolved

#include "Message.h"

namespace Gui {
  Message::Message(const MessageContainer& messageContent, QWidget *parent) : QWidget(parent) {
    messageHSplit = new QHBoxLayout(this);
    Avatar = new QLabel();
    Avatar->setPixmap(messageContent.avatar.scaled(40, 40, Qt::KeepAspectRatio));

    messageInfoVSplit = new QVBoxLayout();
    messageInfoHSplit = new QHBoxLayout();

    senderName = new QLabel(messageContent.senderName);
    time = new QLabel(messageContent.time);
    messageInfoHSplit->addWidget(senderName);
    messageInfoHSplit->addWidget(time);

    message = new QLabel(messageContent.message);
    message->setWordWrap(true); // aktiviert automatischen Zeilenumbruch
    message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    messageInfoVSplit->addLayout(messageInfoHSplit);
    messageInfoVSplit->addWidget(message);

    messageHSplit->addWidget(Avatar);
    messageHSplit->addLayout(messageInfoVSplit);
  }

Message::~Message() {
  delete messageHSplit;
  delete messageInfoHSplit;
  delete messageInfoVSplit;
  delete message;
  delete time;
  delete Avatar;
  delete senderName;
}
} // Gui
