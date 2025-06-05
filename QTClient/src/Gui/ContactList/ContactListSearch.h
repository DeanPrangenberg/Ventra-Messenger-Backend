//
// Created by deanprangenberg on 05.06.25.
//

#ifndef CONTACTLISTSEARCH_H
#define CONTACTLISTSEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCompleter>

#include "ContactButton.h"

namespace Gui {
class ContactListSearch : public QWidget {
Q_OBJECT

public:
    explicit ContactListSearch(QWidget *parent = nullptr);
    ~ContactListSearch() override;
    void connectSearchAndSortToList(const QList<ContactButton*>* ContactButtonList);

private:
    QLineEdit *SearchInField;
    QComboBox *sortInBox;
    QHBoxLayout *SearchLayout;
};
} // Gui

#endif //CONTACTLISTSEARCH_H
