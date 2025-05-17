//
// Created by deanprangenberg on 15.05.25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Sidebar.h" resolved

#include "Sidebar.h"

namespace Gui {
  Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    layout = new QVBoxLayout();
    logoLabel = new QLabel();
    logoLabel->setObjectName("logoLabelSidebar");

    chatButton = new QPushButton();
    chatButton->setObjectName("chatButtonSidebar");

    communityButton = new QPushButton();
    communityButton->setObjectName("communityButtonSidebar");

    settingsButton = new QPushButton();
    settingsButton->setObjectName("settingsButtonSidebar");

    accountButton = new QPushButton();
    accountButton->setObjectName("accountButtonSidebar");

    setLayout(layout);

    updateLayout();
    setupUI();
  }

  void Sidebar::updateLayout() {
    layout->addWidget(logoLabel);
    layout->addStretch();
    layout->addWidget(chatButton);
    layout->addWidget(communityButton);

    for (int i = 0; i < 5; ++i) {
      favList.push_back(new QPushButton());
    }

    for (const auto fav : favList) {
      fav->setObjectName("favSideBarButton");
      layout->addWidget(fav);
    }

    layout->addStretch();
    layout->addWidget(settingsButton);
    layout->addWidget(accountButton);
  }

  Sidebar::~Sidebar() {
    delete layout;
    delete logoLabel;
    delete settingsButton;
    delete chatButton;
    delete communityButton;
    delete accountButton;
  }

  void Sidebar::setupUI() {

    // LOGO
    QPixmap logoPixmap(":/icons/res/logo/DarkText.png");
    if (logoPixmap.isNull()) {
      qDebug() << "Bild konnte nicht geladen werden!";
      return;
    }

    // Logo auf feste Größe skalieren
    logoPixmap = logoPixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    logoLabel->setPixmap(logoPixmap);
    logoLabel->setAlignment(Qt::AlignTop);

    // BUTTONS
    settingsButton->setText("Settings");
    accountButton->setText("Account");
  }
} // Gui
