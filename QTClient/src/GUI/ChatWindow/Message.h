//
// Created by deanprangenberg on 17.05.25.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

namespace Gui {

struct MessageContainer {
    QString message;
    QString time;
    QString senderName;
    QPixmap avatar;
};

class Message : public QWidget {
Q_OBJECT

public:
    explicit Message(const MessageContainer& messageContent, QWidget *parent = nullptr);
    ~Message() override;

private:
    QHBoxLayout *messageHSplit;
    QHBoxLayout *messageInfoHSplit;
    QVBoxLayout *messageInfoVSplit;
    QLabel *message;
    QLabel *time;
    QLabel *Avatar;
    QLabel *senderName;
};
} // Gui

#endif //MESSAGE_H
