//
// Created by deanprangenberg on 17.05.25.
//

#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>
#include <QResizeEvent>

namespace Gui {
  class ContactButton : public QPushButton {
    Q_OBJECT

  public:
    ContactButton(const QString &name, const QPixmap &avatar, QWidget *parent = nullptr);
    ~ContactButton() override;

    void resizeEvent(QResizeEvent *event) override;

    QLabel *avatarLabel;
    QLabel *nameLabel;

  private:
    QPixmap originalAvatar;
  };
} // Gui

#endif //CONTACTWIDGET_H
