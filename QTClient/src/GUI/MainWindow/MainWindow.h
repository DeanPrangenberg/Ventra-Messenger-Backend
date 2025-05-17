//
// Created by deanprangenberg on 15.05.25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QApplication>
#include <QStackedWidget>

namespace Gui {
  class Sidebar;
  
  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    void initializeWidgets();

    void setupLayout();

    void setupStyles();

    void updateStyle(const QString& styleFile);
    ~MainWindow() override;

  private:
    QHBoxLayout *layout;
    Sidebar *sidebar;
    QStackedWidget *screenStack;
    QStackedWidget *chatStack;
    QWidget *chatWidget;
  };
} // Gui

#endif //MAINWINDOW_H
