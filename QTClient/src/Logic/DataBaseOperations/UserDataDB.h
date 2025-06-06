//
// Created by deanprangenberg on 28.05.25.
//

#ifndef USERDATADB_H
#define USERDATADB_H

#include <QList>

#include "../../Database/LocalDatabase.h"
#include "../../Gui/Gui_Structs_Enums.h"

namespace logic {
  class UserDataDB : LocalDatabase {
public:
  UserDataDB(const fs::path &dbPath, const std::string &password, bool debugMode);
  QList<Gui::UserData> getUserData(QString &userUUID);
private:
  bool createUserTables();
  };
}

#endif //USERDATADB_H
