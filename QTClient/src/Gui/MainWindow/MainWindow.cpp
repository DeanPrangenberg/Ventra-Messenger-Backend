#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <iostream>

#include "../Sidebar/Sidebar.h"
#include "../GuiHelper/GuiHelper.h"

namespace Gui {
  class ChatWindow;

  MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Ventra Chat");

    initializeWidgets();
    initStacks();
    setupLayout();
    setupStyles();
    
    switchScreen(ScreenType::CHAT_SCREEN);
  }

  MainWindow::~MainWindow() {
    delete layout;
    delete sidebar;
    delete chatStack;
    delete screenStack;
  }

  void MainWindow::updateStyle(const QString &styleFile) {
    QFile file(styleFile);
    if (!file.open(QFile::ReadOnly)) {
      std::cerr << "Konnte Stylesheet nicht öffnen: " << styleFile.toStdString() << std::endl;
      return;
    }
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);

    // Icons für Buttons aktualisieren
    for (QPushButton *button: findChildren<QPushButton *>()) {
      Gui::GuiHelper::updateButtonIcon(button);
    }
  }

  void MainWindow::switchScreen(ScreenType screenType) {
    switch (screenType) {
      case ScreenType::CHAT_SCREEN:
        screenStack->setCurrentWidget(chatWidget);
        chatStack->setCurrentIndex(static_cast<int>(ChatStackType::DIREKT));
        break;
      case ScreenType::COMMUNITY_SCREEN:
        screenStack->setCurrentWidget(chatWidget);
        chatStack->setCurrentIndex(static_cast<int>(ChatStackType::COMMUNITY));
        break;
      case ScreenType::SETTINGS_SCREEN:
        screenStack->setCurrentWidget(settingsWidget);
        break;
      case ScreenType::ACCOUNT_SCREEN:
        screenStack->setCurrentWidget(accountWidget);
        break;
      default:
        std::cerr << "Ungültiger ScreenType" << std::endl;
        screenStack->setCurrentWidget(chatWidget);
        chatStack->setCurrentIndex(static_cast<int>(ChatStackType::DIREKT));
        break;
    }
  }

  void MainWindow::initializeWidgets() {
    screenStack = new QStackedWidget(this);
    screenStack->setObjectName("screenStack");
    setCentralWidget(screenStack);

    sidebar = new Sidebar();
    sidebar->setObjectName("sidebar");

    chatStack = new QStackedWidget(this);
    chatStack->setObjectName("chatStack");
  }

  void MainWindow::setupLayout() {
    chatWidget = new QWidget(this);
    chatWidget->setObjectName("chatWidget");
    chatWidget->setAutoFillBackground(true);
    layout = new QHBoxLayout(chatWidget);

    layout->addWidget(sidebar);
    layout->addWidget(chatStack, 1);

    screenStack->addWidget(chatWidget);

    screenStack->addWidget(settingsWidget);
    screenStack->addWidget(accountWidget);
  }

  void MainWindow::setupStyles() {
    sidebar->setMaximumWidth(80);
  }

  void MainWindow::initStacks() {
    // Direkt-Chat-screen
    directChatWidget = new DirektChatScreen(this);
    directChatWidget->setObjectName("directChatWidget");
    dmChatManager = new Logic::DMChatManager(directChatWidget, true);
    dmChatManager->updateGuiFromDB();
    chatStack->addWidget(directChatWidget);

    // Community-Chat-Screen
    communityChatWidget = new QWidget(this);
    communityChatWidget->setObjectName("communityChatWidget");
    QHBoxLayout *communityLayout = new QHBoxLayout(communityChatWidget);
    // TODO: Community-Chat-Inhalte hinzufügen
    chatStack->addWidget(communityChatWidget);

    // Einstellungen-screen
    settingsWidget = new QWidget(this);
    settingsWidget->setObjectName("settingsWidget");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsWidget);
    // TODO: Settings-Inhalte hinzufügen

    // Account-screen
    accountWidget = new QWidget(this);
    accountWidget->setObjectName("accountWidget");
    QVBoxLayout *accountLayout = new QVBoxLayout(accountWidget);
    // TODO: Account-Inhalte hinzufügen
  }
} // namespace Gui
