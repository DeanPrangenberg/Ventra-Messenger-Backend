//
// Created by deanprangenberg on 08.05.25.
//

#ifndef LOCALDATABASE_H
#define LOCALDATABASE_H

#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <sqlcipher/sqlite3.h>
#include <openssl/rand.h>
#include <fstream>
#include "../../../Shared/Crypto/KeyEnv/KeyEnv.h"

namespace fs = std::filesystem;

class LocalDatabase {
protected:
  fs::path dbPath;
  std::string password;
  std::vector<uint8_t> salt;     // 16-Byte Salt
  bool debugMode;

  // Helper methods
  void loadOrCreateSalt();
  std::vector<uint8_t> deriveKey() const;
  bool openConnection(sqlite3*& handle) const;
  void closeConnection(sqlite3* handle) const;
  bool createChatTables();

public:
  LocalDatabase(const fs::path& dbPath, const std::string& password, bool debugMode = false);

  bool execute(const std::string& sql);
  bool query(const std::string& sql, std::vector<std::vector<std::string>>& result);
  std::string getLastError() const;
  bool isInDebugMode() const { return debugMode; }
};

#endif // LOCALDATABASE_H
