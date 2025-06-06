//
// Created by deanprangenberg on 17.05.25.
//

#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

#include "ContactButton.h"
#include "ContactListSearch.h"

namespace Gui {
  class ContactList : public QWidget {
    Q_OBJECT

  public:
    explicit ContactList(QWidget *parent = nullptr);

    ContactButton* getContactButtonPointer(const QString& uuid);
    void addContact(const QString &name, const QString &chatUUID, const QPixmap &avatar);
    void removeContact(const QString &chatUUID);
    QList<ContactButton *> contactButtonList;

  private:
    void updateButtonsIcons();
    QVBoxLayout *contactsLayout;
    QVBoxLayout *contactListWidgetLayout;
    QWidget *containerWidget;
    QScrollArea *scrollArea;
    ContactListSearch *contactListSearch;
  };
} // namespace Gui

#endif // CONTACTLIST_H
