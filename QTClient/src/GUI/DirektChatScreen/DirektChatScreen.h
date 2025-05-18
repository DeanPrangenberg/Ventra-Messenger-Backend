//
// Created by deanprangenberg on 18.05.25.
//

#ifndef DIREKTCHATSCREEN_H
#define DIREKTCHATSCREEN_H

#include <QStackedWidget>
#include <QWidget>
#include "../ChatWindow/ChatWindow.h"
#include "../ContactList/ContactList.h"

namespace Gui {
  struct chatData {
    QList<MessageContainer> messageContainerList;
    QString name;
    QString chatUUID;
    QPixmap avatar;
  };

  class DirektChatScreen : public QWidget {
    Q_OBJECT

  public:
    explicit DirektChatScreen(QWidget *parent = nullptr);

    ~DirektChatScreen() override;

    void loadChats(QList<chatData> &chatDataList);

    void removeChat(const QString &chatUUID);

    void addMessagesToChat(const QString &chatUUID, const QList<MessageContainer> &messageContainers);

    void generateAndLoadTestChats(int numChats, int numMessagesPerChat);

    QList<chatData> &getAllChatData();

    QList<chatData> &getChatData(const QString &chatUUID);

  private:
    void addChat(const chatData &chatData);

    void initializeLayout();

    void showChatbyID(const QString &chatUUID);

    QMap<QString, ContactButton *> ButtonMap;
    QMap<QString, ChatWindow *> ChatWindowMap;
    QHBoxLayout *chatScreenLayout;
    ContactList *contactList;
    QStackedWidget *chatWindowStack;
    QWidget *directChatWidget;
  };
} // Gui

#endif //DIREKTCHATSCREEN_H
