//
// Created by deanprangenberg on 25.05.25.
//

#ifndef STRUCTS_ENUMS_H
#define STRUCTS_ENUMS_H

#include <QPixmap>


namespace Gui {
  struct MessageContainer {
    QString chatUUID;
    QString messageUUID;
    QString message;
    QString time;
    QString senderName;
    QString senderUUID;
    QPixmap avatar;
    bool isFollowUp;
  };

  struct chatData {
    QList<MessageContainer> messageContainerList;
    QString name;
    QString chatUUID;
    QPixmap avatar;
  };

  struct UserData {
    QString userUUID;
    QString username;
    QPixmap avatar;
  };
}

#endif //STRUCTS_ENUMS_H
