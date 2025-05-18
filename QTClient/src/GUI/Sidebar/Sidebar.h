//
// Created by deanprangenberg on 15.05.25.
//

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

namespace Gui {

class Sidebar : public QWidget {
Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);
    void updateLayout();
    ~Sidebar() override;

private:
    QVBoxLayout *layout;
    QLabel *logoLabel;
    QPushButton *settingsButton;
    QPushButton *chatButton;
    QPushButton *communityButton;
    QPushButton *accountButton;
    QList<QPushButton*> favList;

    void setupUI();
};
} // Gui

#endif //SIDEBAR_H
