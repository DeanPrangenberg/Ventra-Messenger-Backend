//
// Created by deanprangenberg on 17.05.25.
//

#ifndef CHATINPUTBAR_H
#define CHATINPUTBAR_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace Gui {

class ChatInputBar : public QWidget {
Q_OBJECT

public:
    explicit ChatInputBar(QWidget *parent = nullptr);
    ~ChatInputBar() override;

private:
    QHBoxLayout *layout;
    QLineEdit *chatLineEdit;
    QPushButton *sendButton;
    QPushButton *attachButton;
    QPushButton *audioButton;
};
} // Gui

#endif //CHATINPUTBAR_H
