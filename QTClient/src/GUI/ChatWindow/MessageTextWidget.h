//
// Created by deanprangenberg on 18.05.25.
//

#ifndef MESSAGETEXTLABEL_H
#define MESSAGETEXTLABEL_H

#include <QWidget>
#include <QString>

namespace Gui {
  class MessageTextWidget : public QWidget {
    Q_OBJECT
  public:
    explicit MessageTextWidget(const QString& text, QWidget* parent = nullptr);
    void setText(const QString& text);
    QString text() const;

    // Überschriebene Methoden für dynamische Höhe
    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;

  protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private:
    QString m_text;
  };
} // namespace Gui

#endif //MESSAGETEXTLABEL_H
