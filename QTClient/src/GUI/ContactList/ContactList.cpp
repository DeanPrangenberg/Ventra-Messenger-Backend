//
// Created by deanprangenberg on 17.05.25.
//

#include "ContactList.h"

#include "../GUIHelper/GUIHelper.h"

namespace Gui {
  ContactList::ContactList(QWidget *parent) : QWidget(parent) {
    // Container für die Kontakte
    containerWidget = new QWidget(this);
    contactsLayout = new QVBoxLayout(containerWidget);
    contactsLayout->setAlignment(Qt::AlignTop);
    containerWidget->setLayout(contactsLayout);

    // ScrollArea einrichten
    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Hauptlayout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
  }

  ContactList::~ContactList() = default;

  void ContactList::addContact(const QString& name, const QPixmap& avatar) {
    auto* newContact = new ContactButton(name, avatar, containerWidget);
    contactButtonList.push_back(newContact);

    // Layout vollständig leeren (nicht löschen!)
    QLayoutItem* item;
    while ((item = contactsLayout->takeAt(0)) != nullptr) {
      if (auto* widget = item->widget()) {
        widget->setParent(nullptr); // Widget vom Layout lösen
      }
      delete item; // Speicher vom LayoutItem freigeben
    }

    // Alle Buttons wieder hinzufügen
    for (auto* contact : contactButtonList) {
      contactsLayout->addWidget(contact);
    }

    updateButtonsIcons();
  }

  void ContactList::updateButtonsIcons() {
    for (auto contact : contactButtonList) {
      GUIHelper::updateContactIcon(contact);
    }
  }

} // Gui
