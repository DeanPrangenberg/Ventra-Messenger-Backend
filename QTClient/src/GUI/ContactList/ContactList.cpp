//
// Created by deanprangenberg on 17.05.25.
//

#include "ContactList.h"

#include "../GUIHelper/GUIHelper.h"

namespace Gui {
  ContactList::ContactList(QWidget *parent) : QWidget(parent) {
    containerWidget = new QWidget(this);
    contactsLayout = new QVBoxLayout(containerWidget);
    contactsLayout->setAlignment(Qt::AlignTop);
    contactsLayout->setContentsMargins(0, 0, 0, 0);
    containerWidget->setLayout(contactsLayout);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);

    // Use Minimum for horizontal to allow proper sizing
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  }

  ContactList::~ContactList() = default;

  void ContactList::addContact(const QString &name, const QString &chatUUID, const QPixmap &avatar) {
    auto *newContact = new ContactButton(name, avatar, containerWidget);
    contactButtonList.push_back(newContact);

    // Clear layout
    QLayoutItem *item;
    while ((item = contactsLayout->takeAt(0)) != nullptr) {
      if (auto *widget = item->widget()) {
        widget->setParent(nullptr);
      }
      delete item;
    }

    // Add all buttons and find max width
    int maxWidth = 0;
    for (auto *contact: contactButtonList) {
      contact->adjustSize();
      int width = contact->sizeHint().width() + 40;
      maxWidth = qMax(maxWidth, width);
      contactsLayout->addWidget(contact);
    }

    // Set minimum width for scroll area and container
    scrollArea->setMinimumWidth(maxWidth + 30);
    containerWidget->setMinimumWidth(maxWidth);

    // Update the layout
    adjustSize();
    if (parentWidget()) {
      parentWidget()->adjustSize();
    }

    updateButtonsIcons();
  }

  void ContactList::updateButtonsIcons() {
    for (auto contact: contactButtonList) {
      GUIHelper::updateContactIcon(contact);
    }
  }
} // Gui
