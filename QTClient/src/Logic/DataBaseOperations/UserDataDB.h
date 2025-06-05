//
// Created by deanprangenberg on 28.05.25.
//

#ifndef USERDATADB_H
#define USERDATADB_H

#include "../../Database/LocalDatabase.h"

namespace logic {
  class UserDataDB : LocalDatabase {
public:
  UserDataDB(const fs::path &dbPath, const std::string &password, bool debugMode);
   getUserData();
private:
  bool createUserTables();
  };
}

#endif //USERDATADB_H
