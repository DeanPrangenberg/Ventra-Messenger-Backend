#include "GUIHelper.h"

namespace Gui {
  void GUIHelper::updateButtonIcon(QPushButton *button) {
    if (!button) return;

    QIcon icon = button->icon();
    if (icon.isNull()) return;

    QSize btnSize = button->size();

    QSize scaledSize = icon.actualSize(btnSize);
    scaledSize.scale(btnSize, Qt::KeepAspectRatio);

    button->setIconSize(scaledSize);
  }
} // namespace Gui
