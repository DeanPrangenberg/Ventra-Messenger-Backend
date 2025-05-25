#include "Message.h"

namespace Gui {
  Message::Message(const MessageContainer &messageContent, QWidget *parent) : QWidget(parent) {
    messageHSplit = new QHBoxLayout(this);
    messageHSplit->setContentsMargins(0, 0, 0, 0);
    messageHSplit->setSpacing(8);

    // Avatar oder Platzhalter
    Avatar = new QLabel();
    Avatar->setObjectName("MessageAvatar");
    Avatar->setFixedSize(40, 40);

    // Hauptbereich rechts vom Avatar
    QWidget *rightWidget = new QWidget(this);
    rightWidget->setObjectName("MessageRightWidget");

    messageInfoVSplit = new QVBoxLayout(rightWidget);
    messageInfoVSplit->setSpacing(0);
    messageInfoVSplit->setContentsMargins(0, 0, 0, 0);

    if (messageContent.isFollowUp) {
      Avatar->setStyleSheet("background-color: transparent;");
    } else {
      Avatar->setPixmap(messageContent.avatar.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

      QWidget *topRowWidget = new QWidget(rightWidget);
      topRowWidget->setObjectName("MessageTopRow");
      topRowWidget->setFixedHeight(16);

      messageInfoHSplit = new QHBoxLayout(topRowWidget);
      messageInfoHSplit->setContentsMargins(0, 0, 0, 0);
      messageInfoHSplit->setSpacing(0);

      senderName = new QLabel(messageContent.senderName, topRowWidget);
      senderName->setObjectName("MessageSenderName");
      senderName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

      time = new QLabel(messageContent.time, topRowWidget);
      time->setObjectName("MessageTime");
      time->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      messageInfoHSplit->addWidget(senderName);
      messageInfoHSplit->addStretch();
      messageInfoHSplit->addWidget(time);

      messageInfoVSplit->addWidget(topRowWidget);
    }

    message = new MessageTextWidget(messageContent.message, rightWidget);
    message->setObjectName("MessageText");
    messageInfoVSplit->addWidget(message);

    messageHSplit->addWidget(Avatar, 0, Qt::AlignTop);
    messageHSplit->addWidget(rightWidget);

    setContentsMargins(0, 0, 0, 0);
  }

  Message::~Message() {
    delete messageHSplit;
    if (messageInfoHSplit) delete messageInfoHSplit;
    delete messageInfoVSplit;
    delete message;
    if (time) delete time;
    delete Avatar;
    if (senderName) delete senderName;
  }
} // Gui
