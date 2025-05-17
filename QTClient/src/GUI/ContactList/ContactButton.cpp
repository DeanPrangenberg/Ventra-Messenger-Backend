//
// Created by deanprangenberg on 17.05.25.
//

#include "ContactButton.h"

namespace Gui {
  ContactButton::ContactButton(const QString &name, const QPixmap &avatar, QWidget *parent)
      : QPushButton(parent), originalAvatar(avatar) {
    auto *layout = new QHBoxLayout(this);

    avatarLabel = new QLabel(this);
    avatarLabel->setPixmap(avatar);
    avatarLabel->setFixedSize(24, 24); // initiale Größe
    avatarLabel->setScaledContents(true);

    nameLabel = new QLabel(name, this);
    layout->addWidget(avatarLabel);
    layout->addWidget(nameLabel);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    setLayout(layout);

    setFixedHeight(50);
  }

  void ContactButton::resizeEvent(QResizeEvent *event) {
    QPushButton::resizeEvent(event);

    int iconHeight = height() - 16; // z.B. 30 - (2 * 8 Margin)
    int iconSize = qMax(16, iconHeight);

    QPixmap scaled = originalAvatar.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    avatarLabel->setPixmap(scaled);
    avatarLabel->setFixedSize(scaled.size());
  }

  ContactButton::~ContactButton() {
    delete avatarLabel;
    delete nameLabel;
  }
} // Gui
