//
// Created by deanprangenberg on 15.05.25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QApplication>
#include <QStackedWidget>

#include "../DirektChatScreen/DirektChatScreen.h"
#include "../../Logic/ScreenManager/DMChatManager.h"
#include "../Sidebar/Sidebar.h"

namespace Gui {

  enum class ScreenType {
    CHAT_SCREEN,
    COMMUNITY_SCREEN,
    SETTINGS_SCREEN,
    ACCOUNT_SCREEN,
  };

  enum class ChatStackType {
    DIREKT = 0,
    COMMUNITY = 1,
  };

  enum class ScreenStackType {
    CHAT = 0,
    SETTINGS = 1,
    ACCOUNT = 2,
  };

  class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

    void updateStyle(const QString &styleFile);

    void switchScreen(ScreenType screenType);

    ~MainWindow() override;

  private:
    void initStacks();

    void initializeWidgets();

    void setupLayout();

    void setupStyles();

    QHBoxLayout *layout;
    Sidebar *sidebar;

    QStackedWidget *screenStack;
    QStackedWidget *chatStack;

    QWidget *chatWidget;
    DirektChatScreen *directChatWidget;
    Logic::DMChatManager *dmChatManager;
    QWidget *communityChatWidget;

    QWidget *settingsWidget;
    QWidget *accountWidget;
  };
} // Gui

#endif //MAINWINDOW_H
