//
// Created by deanprangenberg on 05.06.25.
//


#include "ContactListSearch.h"

namespace Gui {
  ContactListSearch::ContactListSearch(QWidget *parent) : QWidget(parent) {
    SearchLayout = new QHBoxLayout(this);
    SearchInField = new QLineEdit();
    sortInBox = new QComboBox();

    setLayout(SearchLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setContentsMargins(0, 0, 0, 0);
    SearchLayout->setContentsMargins(0, 0, 0, 0);
    SearchLayout->setSpacing(0);

    SearchLayout->addWidget(SearchInField, 4);
    SearchLayout->addWidget(sortInBox, 1);

    SearchInField->setPlaceholderText("Search...");

    sortInBox->addItem("Normal");
    sortInBox->addItem("A -> Z");
    sortInBox->addItem("Z -> A");
  }

  ContactListSearch::~ContactListSearch() {
    delete sortInBox;
    delete SearchInField;
    delete SearchLayout;
  }

  void ContactListSearch::connectSearchAndSortToList(const QList<ContactButton *> *ContactButtonList) {
    // Search functionality
    connect(SearchInField, &QLineEdit::textChanged, this, [ContactButtonList, this]() {
      QString searchText = SearchInField->text();
      for (ContactButton *btn: *ContactButtonList) {
        bool match = btn->nameLabel->text().contains(searchText, Qt::CaseInsensitive);
        btn->setVisible(match);
      }
    });

    // Sort functionality
    connect(sortInBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [ContactButtonList](int index) {
              QList<ContactButton *> tempList = *ContactButtonList;

              switch (index) {
                case 1: // A -> Z
                  std::sort(tempList.begin(), tempList.end(),
                            [](ContactButton *a, ContactButton *b) {
                              return a->nameLabel->text() < b->nameLabel->text();
                            });
                  break;
                case 2: // Z -> A
                  std::sort(tempList.begin(), tempList.end(),
                            [](ContactButton *a, ContactButton *b) {
                              return a->nameLabel->text() > b->nameLabel->text();
                            });
                  break;
                default: // Normal
                  break;
              }

              if (QLayout *parentLayout = tempList[0]->parentWidget()->layout()) {
                for (ContactButton *btn: tempList) {
                  parentLayout->removeWidget(btn);
                }
                for (ContactButton *btn: tempList) {
                  parentLayout->addWidget(btn);
                }
              }
            });
  }
} // Gui
