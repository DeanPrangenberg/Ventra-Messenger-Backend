//
// Created by deanprangenberg on 15.05.25.
//

#include "MainWindow.h"

#include <QFile>

#include "../Sidebar/Sidebar.h"
#include "../GUIHelper/GUIHelper.h"

namespace Gui {
  MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Ventra Chat");

    initializeWidgets();
    setupLayout();
    setupStyles();
  }

  void MainWindow::initializeWidgets() {
    // Zentraler Bildschirm-Stack
    screenStack = new QStackedWidget(this);
    screenStack->setObjectName("screenStack");
    setCentralWidget(screenStack);

    // Seitenleiste
    sidebar = new Sidebar();
    sidebar->setObjectName("sidebar");

    // Chat-Bereich
    chatStack = new QStackedWidget(this);
    chatStack->setObjectName("chatStack");
  }

  void MainWindow::setupLayout() {
    // Haupt-Chat-Widget und horizontales Layout
    chatWidget = new QWidget(this);
    chatWidget->setObjectName("chatWidget");
    layout = new QHBoxLayout(chatWidget);

    layout->addWidget(sidebar);
    layout->addWidget(chatStack);

    screenStack->addWidget(chatWidget);
  }

  void MainWindow::setupStyles() {
    sidebar->setMaximumWidth(80);
  }

  void MainWindow::updateStyle(const QString &styleFile) {
    QFile file(styleFile);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);

    for (QPushButton* button : this->findChildren<QPushButton*>()) {
      Gui::GUIHelper::updateButtonIcon(button);
    }

  }

  MainWindow::~MainWindow() {
    delete layout;
    delete sidebar;
    delete chatStack;
    delete screenStack;
    delete chatWidget;
  }
} // Gui
