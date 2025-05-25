//
// Created by deanprangenberg on 17.05.25.
//

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

#include "ChatInputBar.h"
#include "Message.h"
#include "../GuiHelper/GuiHelper.h"

namespace Gui {
  class ChatWindow : public QWidget {
    Q_OBJECT

  public:
    explicit ChatWindow(const QString chatUUIDIn, QWidget *parent = nullptr);
    ~ChatWindow() override;
    void setChatHistory(const QList<MessageContainer> &messageListIn);
    QList<MessageContainer>& getChatHistory();
    void addNewMessages(QList<MessageContainer> messageContainers);
    void addOldMessages(QList<MessageContainer> messageContainers);
    QString chatUUID;
    QList<MessageContainer> messageList;

  private:
    void updateDisplayedMessage();
    QWidget* messageContainer;
    QVBoxLayout *messageContainerLayout;
    QVBoxLayout *WindowLayout;
    QScrollArea *chatArea;
    ChatInputBar *chatInputBar;
  };
} // Gui

#endif //CHATWINDOW_H
