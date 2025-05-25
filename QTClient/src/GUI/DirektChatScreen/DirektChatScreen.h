//
// Created by deanprangenberg on 18.05.25.
//

#ifndef DIREKTCHATSCREEN_H
#define DIREKTCHATSCREEN_H

#include <QStackedWidget>
#include <QWidget>
#include <QMutexLocker>
#include "../ChatWindow/ChatWindow.h"
#include "../ContactList/ContactList.h"
#include "../../Database/DmChatDB.h"
#include "../../Crypto/IDs/GenerateID.h"
#include "../../ThreadPool/ThreadPool.h"

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

    void loadChatsFromDB();

    static bool saveToDatabase(const QString& dbPath, const QList<chatData>& allChatData);

  public slots:  // Add this section
    void addChat(const chatData &data);

  private:
    void initializeLayout();
    void showChatbyID(const QString &chatUUID);
    void updateDebugInfo();

    std::atomic<int> pendingSaves{0};
    std::atomic<bool> isLoading{false};
    QMutex dbMutex;
    QList<chatData> chatDataCache;
    QMap<QString, ContactButton *> ButtonMap;
    QMap<QString, ChatWindow *> ChatWindowMap;
    QHBoxLayout *chatScreenLayout;
    ContactList *contactList;
    QStackedWidget *chatWindowStack;
    QWidget *directChatWidget;
    DmChatDB db;
  };
} // Gui

#endif //DIREKTCHATSCREEN_H
