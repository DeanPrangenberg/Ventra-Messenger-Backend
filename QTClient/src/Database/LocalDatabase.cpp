//
// Created by deanprangenberg on 08.05.25.
//

#include "LocalDatabase.h"

#include "../../../Shared/Crypto/KDF/KDFEnv.h"

LocalDatabase::LocalDatabase(const fs::path &dbPath, const std::string &password, bool debugMode)
    : dbPath(dbPath), password(password), debugMode(debugMode) {
  if (dbPath.empty() || (!debugMode && password.empty())) {
    throw std::runtime_error("DB path or password empty in encrypted mode");
  }
  if (!debugMode) {
    loadOrCreateSalt();
  }
}

void LocalDatabase::loadOrCreateSalt() {
  fs::path saltFile = dbPath;
  saltFile += ".salt";

  if (fs::exists(saltFile)) {
    // Datei öffnen
    std::ifstream in(saltFile, std::ios::binary);
    if (!in) {
      throw std::runtime_error("Could not open salt file for reading");
    }

    salt.assign(
      std::istreambuf_iterator<char>(in),
      std::istreambuf_iterator<char>()
    );
  } else {
    salt.resize(16);
    if (!RAND_bytes(salt.data(), salt.size())) {
      throw std::runtime_error("Could not generate salt");
    }

    std::ofstream out(saltFile, std::ios::binary);
    if (!out) {
      throw std::runtime_error("Could not open salt file for writing");
    }
    out.write(reinterpret_cast<const char *>(salt.data()), salt.size());
  }
}

std::vector<uint8_t> LocalDatabase::deriveKey() const {
  // 32-Byte key für AES-256
  std::vector<uint8_t> key(32);
  std::vector<uint8_t> ikm(password.begin(), password.end());
  std::string info = "SQLiteCipherDBPassword";
  Crypto::KDFEnv kdf(Crypto::KDFType::SHA3_256);
  kdf.startKDF(ikm, salt, info, key, key.size());
  return key;
}

bool LocalDatabase::openConnection(sqlite3 *&handle) const {
  int rc;

  if (debugMode) {
    // Open database in debug mode (unencrypted)
    rc = sqlite3_open_v2(dbPath.c_str(), &handle,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                        nullptr);
    std::cout << "Opening database in DEBUG mode (unencrypted)" << std::endl;
  } else {
    // Open database in encrypted mode
    rc = sqlite3_open_v2(dbPath.c_str(), &handle,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                        nullptr);
    if (rc != SQLITE_OK) return false;

    auto key = deriveKey();
    rc = sqlite3_key(handle, key.data(), static_cast<int>(key.size()));
    std::fill(key.begin(), key.end(), 0);
  }

  return rc == SQLITE_OK;
}

void LocalDatabase::closeConnection(sqlite3 *handle) const {
  sqlite3_close(handle);
}

bool LocalDatabase::execute(const std::string &sql) {
  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return false;

  char *errMsg = nullptr;
  int rc = sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL-error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  }

  closeConnection(handle);
  return rc == SQLITE_OK;
}

bool LocalDatabase::query(const std::string &sql,
                          std::vector<std::vector<std::string> > &result) {
  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return false;

  char *errMsg = nullptr;
  int rc = sqlite3_exec(handle, sql.c_str(),
                        [](void *data, int argc, char **argv, char **azColName) -> int {
                          auto *res = reinterpret_cast<std::vector<std::vector<std::string> > *>(data);
                          std::vector<std::string> row;
                          for (int i = 0; i < argc; ++i)
                            row.push_back(argv[i] ? argv[i] : "NULL");
                          res->push_back(row);
                          return 0;
                        },
                        &result, &errMsg);

  if (rc != SQLITE_OK) {
    std::cerr << "SQL-error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  }

  closeConnection(handle);
  return rc == SQLITE_OK;
}

std::string LocalDatabase::getLastError() const {
  return sqlite3_errmsg(nullptr); // wenn nötig, kann man letzten Handle cachen
}
