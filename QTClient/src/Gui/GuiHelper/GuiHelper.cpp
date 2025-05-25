#include "GuiHelper.h"

namespace Gui {
  void GuiHelper::updateButtonIcon(QPushButton *button) {
    if (!button) return;

    QIcon icon = button->icon();
    if (icon.isNull()) return;

    QSize btnSize = button->size();

    QSize scaledSize = icon.actualSize(btnSize);
    scaledSize.scale(btnSize, Qt::KeepAspectRatio);

    button->setIconSize(scaledSize);
  }

  void GuiHelper::updateContactIcon(ContactButton *contactButton) {
    if (!contactButton) return;

    // Button-Größe
    QSize btnSize = contactButton->size();

    // Originalbild aus avatarLabel holen
    QPixmap pixmap = contactButton->avatarLabel->pixmap(Qt::ReturnByValue);
    if (pixmap.isNull()) return;

    // Bild passend zur Button-Größe skalieren (mit Aspect Ratio)
    QPixmap scaledPixmap = pixmap.scaled(btnSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    contactButton->avatarLabel->setPixmap(scaledPixmap);

    // avatarLabel in die Mitte setzen
    contactButton->avatarLabel->setFixedSize(scaledPixmap.size());
    contactButton->avatarLabel->move((btnSize.width() - scaledPixmap.width()) / 2,
                                     (btnSize.height() - scaledPixmap.height()) / 2);
  }

  void GuiHelper::clearLayout(QLayout* layout) {
    if (!layout) return;

    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
      if (QWidget* widget = item->widget()) {
        widget->setParent(nullptr);
        widget->deleteLater();
      } else if (QLayout* childLayout = item->layout()) {
        clearLayout(childLayout);
        delete childLayout;
      }
      delete item;
    }
  }
} // namespace Gui
