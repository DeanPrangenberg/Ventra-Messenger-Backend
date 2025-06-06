#include "DirektChatScreen.h"
#include <iostream>

namespace Gui {
  DirektChatScreen::DirektChatScreen(QWidget *parent)
    : QWidget(parent) {
    initializeLayout();
  }

  DirektChatScreen::~DirektChatScreen() {
    // Clean up maps
    qDeleteAll(chatWindowMap);
    chatWindowMap.clear();
    ButtonMap.clear();

    delete chatScreenLayout;
    delete contactList;
    delete chatWindowStack;
  }

  void DirektChatScreen::initializeLayout() {
    // Setup main layout
    chatScreenLayout = new QHBoxLayout(this);
    chatScreenLayout->setObjectName("DMScreenLayout");
    chatScreenLayout->setContentsMargins(0, 0, 0, 0);

    // Initialize contact list
    contactList = new ContactList(this);
    contactList->setObjectName("DMScreenContactList");
    contactList->setMinimumWidth(250);
    contactList->setMaximumWidth(350);
    contactList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Initialize chat window stack
    chatWindowStack = new QStackedWidget(this);
    chatWindowStack->setObjectName("DMScreenChatWindowStack");

    // Add widgets to layout
    chatScreenLayout->addWidget(contactList);
    chatScreenLayout->addWidget(chatWindowStack, 1);
    chatScreenLayout->setSpacing(0);

    setLayout(chatScreenLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  void DirektChatScreen::showChatbyID(const QString &chatUUID) {
    auto *chatWindow = chatWindowMap.value(chatUUID);
    if (!chatWindow) {
      std::cerr << "Chat window not found for UUID: " << chatUUID.toStdString() << std::endl;
      return;
    }

    // Uncheck all buttons and check the selected one
    for (auto *button: ButtonMap.values()) {
      button->setChecked(button->chatUUID == chatUUID);
    }

    chatWindowStack->setCurrentWidget(chatWindow);
  }

}
