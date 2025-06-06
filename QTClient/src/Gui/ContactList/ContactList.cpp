//
// Created by deanprangenberg on 17.05.25.
//

#include "ContactList.h"

#include "../GuiHelper/GuiHelper.h"

namespace Gui {
  ContactList::ContactList(QWidget *parent) : QWidget(parent) {
    containerWidget = new QWidget(this);
    containerWidget->setObjectName("ContactListWidget");
    containerWidget->setContentsMargins(0, 0, 0, 0);

    contactsLayout = new QVBoxLayout(containerWidget);
    contactsLayout->setAlignment(Qt::AlignTop);
    contactsLayout->setContentsMargins(0, 0, 0, 0);
    containerWidget->setLayout(contactsLayout);

    contactListSearch = new ContactListSearch(this);
    contactListSearch->connectSearchAndSortToList(&contactButtonList);

    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("ContactListScrollArea");
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    contactListWidgetLayout = new QVBoxLayout(this);
    contactListWidgetLayout->setContentsMargins(0, 0, 0, 0);
    contactListWidgetLayout->setSpacing(0);
    contactListWidgetLayout->addWidget(contactListSearch);
    contactListWidgetLayout->addWidget(scrollArea);
    setLayout(contactListWidgetLayout);
    setContentsMargins(0, 0, 0, 0);

    // Use Minimum for horizontal to allow proper sizing
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  }

  ContactButton *ContactList::getContactButtonPointer(const QString &uuid) {
    for (auto *contact: contactButtonList) {
      if (contact->chatUUID == uuid) {
        return contact;
      }
    }
    return nullptr;
  }

  void ContactList::addContact(const QString &name, const QString &chatUUID, const QPixmap &avatar) {
    auto *newContact = new ContactButton(name, chatUUID, avatar, containerWidget);
    newContact->setObjectName("ContactListContactButton");
    contactButtonList.push_back(newContact);

    // Clear layout
    QLayoutItem *item;
    while ((item = contactsLayout->takeAt(1)) != nullptr) {
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

  void ContactList::removeContact(const QString &chatUUID) {
    auto it = std::remove_if(contactButtonList.begin(), contactButtonList.end(),
                             [&chatUUID](ContactButton *contact) {
                               return contact->chatUUID == chatUUID;
                             });

    if (it != contactButtonList.end()) {
      delete *it;
      contactButtonList.erase(it);
      updateButtonsIcons();
    }
  }

  void ContactList::updateButtonsIcons() {
    for (auto contact: contactButtonList) {
      GuiHelper::updateContactIcon(contact);
    }
  }
} // Gui
