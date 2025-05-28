#include "MessageTextWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QStyleOption>
#include <QResizeEvent>

namespace Gui {
  MessageTextWidget::MessageTextWidget(const QString& text, QWidget* parent)
      : QWidget(parent), m_text(text) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setContentsMargins(0, 0, 0, 0); // Keine Ränder
  }

  void MessageTextWidget::setText(const QString& text) {
    m_text = text;
    updateGeometry(); // Neuberechnung der Geometrie auslösen
    update();
  }

  QString MessageTextWidget::text() const {
    return m_text;
  }

  bool MessageTextWidget::hasHeightForWidth() const {
    return true; // Aktiviert Höhenanpassung basierend auf der Breite
  }

  int MessageTextWidget::heightForWidth(int w) const {
    QFontMetrics fm(font());
    // Berechnet die benötigte Höhe für die gegebene Breite mit Word-Wrap
    QRect rect = fm.boundingRect(QRect(0, 0, w, 0), Qt::TextWordWrap, m_text);
    return rect.height();
  }

  void MessageTextWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateGeometry(); // Aktualisiert die Geometrie bei Größenänderung
  }

  // In MessageTextWidget::paintEvent
  void MessageTextWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStyleOption opt;
    opt.initFrom(this); // Initialisiert die Option mit den Eigenschaften des Widgets
    QPainter painter(this);

    // Zeichnet den Hintergrund gemäß dem Stylesheet (inkl. background-color)
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    // Zeichnet den Text mit Word-Wrap und ohne Ränder
    painter.drawText(rect(), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, m_text);
  }
} // namespace Gui