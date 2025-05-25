#ifndef MESSAGE_H
#define MESSAGE_H

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include "MessageTextWidget.h"
#include "../Gui_Structs_Enums.h"

namespace Gui {
  class Message : public QWidget {
    Q_OBJECT

  public:
    explicit Message(const MessageContainer &messageContent, QWidget *parent = nullptr);
    ~Message() override;
    QList<MessageContainer> getAllMessages();
    void addNewMessage(MessageContainer messageContainer);

  private:
    QHBoxLayout *messageHSplit;
    QHBoxLayout *messageInfoHSplit = nullptr;
    QVBoxLayout *messageInfoVSplit;
    MessageTextWidget *message;
    QLabel *time = nullptr;
    QLabel *Avatar;
    QLabel *senderName = nullptr;
  };
} // Gui

#endif //MESSAGE_H
