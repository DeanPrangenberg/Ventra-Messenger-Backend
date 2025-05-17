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

namespace Gui {
  class ChatWindow : public QWidget {
    Q_OBJECT

  public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow() override;

  private:
    QVector<Message*> inputBars;
    QWidget* messageContainer;
    QVBoxLayout *messageContainerLayout;
    QVBoxLayout *WindowLayout;
    QScrollArea *chatArea;
    ChatInputBar *chatInputBar;
  };
} // Gui

#endif //CHATWINDOW_H
