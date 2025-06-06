//
// Created by deanprangenberg on 28.05.25.
//

#include "UserDataDB.h"

namespace logic {
  UserDataDB::UserDataDB(const fs::path &dbPath, const std::string &password, bool debugMode)
      : LocalDatabase(dbPath, password, debugMode) {
    if (createUserTables()) {
      std::cout << "User tables created successfully" << std::endl;
      if (debugMode) {
        std::cout << "Running in DEBUG mode (unencrypted database)" << std::endl;
      }
    } else {
      std::cerr << "Error creating user tables: " << getLastError() << std::endl;
    }
  }

  QList<Gui::UserData> UserDataDB::getUserData(QString &userUUID) {

  }

  bool UserDataDB::createUserTables() {
    std::string createUsersTable =
        "CREATE TABLE IF NOT EXISTS users ("
        "user_uuid TEXT PRIMARY KEY,"
        "username TEXT NOT NULL,"
        "avatar BLOB"
        ");";

    return execute(createUsersTable);
  }
}