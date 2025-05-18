//
// Created by deanprangenberg on 17.05.25.
//

#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

#include "ContactButton.h"

namespace Gui {
  class ContactList : public QWidget {
    Q_OBJECT

  public:
    explicit ContactList(QWidget *parent = nullptr);

    ~ContactList() override;
    ContactButton* getContactButtonPointer(const QString& uuid);
    void addContact(const QString &name, const QString &chatUUID, const QPixmap &avatar);

  private:
    void updateButtonsIcons();

    QList<ContactButton *> contactButtonList;
    QVBoxLayout *contactsLayout;
    QWidget *containerWidget;
    QScrollArea *scrollArea;
  };
} // namespace Gui

#endif // CONTACTLIST_H
