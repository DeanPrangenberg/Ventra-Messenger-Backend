//
// Created by deanprangenberg on 17.05.25.
//

#ifndef GUIHELPER_H
#define GUIHELPER_H

#include <QPushButton>
#include <QIcon>

#include "../ContactList/ContactButton.h"

namespace Gui {
  class GUIHelper {
  public:
    static void updateButtonIcon(QPushButton *button);
    static void updateContactIcon(ContactButton *contactButton);

  private:
  };
} // Gui

#endif //GUIHELPER_H
