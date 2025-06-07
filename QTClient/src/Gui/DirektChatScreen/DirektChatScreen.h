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
#include "../../../../Shared/Crypto/IDs/GenerateID.h"
#include "../../ThreadPool/ThreadPool.h"
#include "../Gui_Structs_Enums.h"
#include "../../Logic/GuiUpdates/DMChatGuiManager.h"

namespace Logic {
  class DMChatGuiManager;
}

namespace Gui {
  class DirektChatScreen : public QWidget {
    Q_OBJECT
    friend class Logic::DMChatGuiManager;
  public:
    explicit DirektChatScreen(QWidget *parent = nullptr);
    ~DirektChatScreen() override;

  private:
    void initializeLayout();
    void showChatbyID(const QString &chatUUID);

    QMap<QString, ContactButton *> ButtonMap;
    QMap<QString, ChatWindow *> chatWindowMap;
    QHBoxLayout *chatScreenLayout;
    ContactList *contactList;
    QStackedWidget *chatWindowStack;
    QWidget *directChatWidget;
  };
} // Gui

#endif //DIREKTCHATSCREEN_H
