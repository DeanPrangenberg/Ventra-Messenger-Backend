//
// Created by deanprangenberg on 17.05.25.
//

#include "ContactButton.h"

namespace Gui {
  ContactButton::ContactButton(const QString &name, const QString& chatUuid, const QPixmap &avatar, QWidget *parent)
    : QPushButton(parent), originalAvatar(avatar) {
    setCheckable(true);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    chatUUID = chatUuid;

    avatarLabel = new QLabel(this);
    avatarLabel->setPixmap(avatar);
    avatarLabel->setFixedSize(24, 24);
    avatarLabel->setScaledContents(true);

    nameLabel = new QLabel(name, this);
    nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    nameLabel->adjustSize();

    layout->addWidget(avatarLabel);
    layout->addWidget(nameLabel);
    setLayout(layout);

    setFixedHeight(50);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    adjustSize();
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
