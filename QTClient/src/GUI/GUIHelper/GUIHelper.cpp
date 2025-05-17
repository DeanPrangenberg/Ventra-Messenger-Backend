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

  void GUIHelper::updateContactIcon(ContactButton *contactButton) {
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
} // namespace Gui
