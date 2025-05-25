//
// Created by deanprangenberg on 22.05.25.
//

#include "DmChatDB.h"

DmChatDB::DmChatDB(const fs::path &dbPath, const std::string &password, bool debugMode)
  : LocalDatabase(dbPath, password, debugMode) {
  if (createChatTables()) {
    std::cout << "Tables created successfully" << std::endl;
    if (debugMode) {
      std::cout << "Running in DEBUG mode (unencrypted database)" << std::endl;
    }
  } else {
    std::cerr << "Error creating tables: " << getLastError() << std::endl;
  }
}

bool DmChatDB::createChatTables() {
  std::string createChatsTable =
      "CREATE TABLE IF NOT EXISTS chats ("
      "chat_uuid TEXT PRIMARY KEY,"
      "name TEXT NOT NULL,"
      "avatar BLOB"
      ");";

  std::string createMessagesTable =
      "CREATE TABLE IF NOT EXISTS messages ("
      "message_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "chat_uuid TEXT,"
      "sender_uuid TEXT NOT NULL,"
      "content TEXT NOT NULL,"
      "timestamp TEXT NOT NULL,"
      "sender_name TEXT NOT NULL,"
      "sender_avatar BLOB,"
      "is_history INTEGER,"
      "FOREIGN KEY (chat_uuid) REFERENCES chats(chat_uuid)"
      ");";

  return execute(createChatsTable) && execute(createMessagesTable);
}

bool DmChatDB::insertChat(const QString &chatUUID, const QString &name, const QPixmap &avatar) {
  std::string sql = "INSERT OR REPLACE INTO chats (chat_uuid, name, avatar) VALUES (?, ?, ?);";

  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return false;

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    // Convert avatar to PNG data
    QByteArray avatarData;
    QBuffer buffer(&avatarData);
    buffer.open(QIODevice::WriteOnly);
    avatar.save(&buffer, "PNG");
    buffer.close();

    // Convert strings to UTF-8
    QByteArray chatUuidUtf8 = chatUUID.toUtf8();
    QByteArray nameUtf8 = name.toUtf8();

    // Bind values
    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, nameUtf8.constData(), nameUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
      std::cerr << "Error inserting chat: " << sqlite3_errmsg(handle) << std::endl;
    }
  }

  sqlite3_finalize(stmt);
  closeConnection(handle);
  return rc == SQLITE_DONE;
}

bool DmChatDB::insertMessage(const QString &chatUUID, const Gui::MessageContainer &msg) {
  std::string sql =
      "INSERT INTO messages "
      "(chat_uuid, sender_uuid, content, timestamp, sender_name, sender_avatar, is_history) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return false;

  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    // Convert avatar to PNG data
    QByteArray avatarData;
    QBuffer buffer(&avatarData);
    buffer.open(QIODevice::WriteOnly);
    msg.avatar.save(&buffer, "PNG");
    buffer.close();

    // Convert strings to UTF-8
    QByteArray chatUuidUtf8 = chatUUID.toUtf8();
    QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
    QByteArray contentUtf8 = msg.message.toUtf8();
    QByteArray timestampUtf8 = msg.time.toUtf8();
    QByteArray senderNameUtf8 = msg.senderName.toUtf8();

    // Bind values
    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, senderUuidUtf8.constData(), senderUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contentUtf8.constData(), contentUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestampUtf8.constData(), timestampUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, senderNameUtf8.constData(), senderNameUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 6, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, msg.isFollowUp ? 1 : 0);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
      std::cerr << "Error inserting message: " << sqlite3_errmsg(handle) << std::endl;
    }
  }

  sqlite3_finalize(stmt);
  closeConnection(handle);
  return rc == SQLITE_DONE;
}

bool DmChatDB::insertMessages(const QString &chatUUID, const QList<Gui::MessageContainer> &messages) {
  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return false;

  if (sqlite3_exec(handle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr) != SQLITE_OK) {
    closeConnection(handle);
    return false;
  }

  std::string sql =
      "INSERT INTO messages "
      "(chat_uuid, sender_uuid, content, timestamp, sender_name, sender_avatar, is_history) "
      "VALUES (?, ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_exec(handle, "ROLLBACK", nullptr, nullptr, nullptr);
    closeConnection(handle);
    return false;
  }

  bool success = true;
  for (const auto &msg: messages) {
    QByteArray avatarData; {
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      msg.avatar.save(&buffer, "PNG");
      buffer.close();
    }

    QByteArray chatUuidUtf8 = chatUUID.toUtf8();
    QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
    QByteArray contentUtf8 = msg.message.toUtf8();
    QByteArray timestampUtf8 = msg.time.toUtf8();
    QByteArray senderNameUtf8 = msg.senderName.toUtf8();

    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, senderUuidUtf8.constData(), senderUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contentUtf8.constData(), contentUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestampUtf8.constData(), timestampUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, senderNameUtf8.constData(), senderNameUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 6, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, msg.isFollowUp ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "SQLite error: " << sqlite3_errmsg(handle) << std::endl;
      success = false;
      break;
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
  }

  sqlite3_finalize(stmt);

  if (success) {
    sqlite3_exec(handle, "COMMIT", nullptr, nullptr, nullptr);
  } else {
    sqlite3_exec(handle, "ROLLBACK", nullptr, nullptr, nullptr);
  }

  closeConnection(handle);
  return success;
}

QList<DmChatDB::ChatInfo> DmChatDB::getAllChats() {
  QList<ChatInfo> chats;
  const char *sql = "SELECT chat_uuid, name, avatar FROM chats;";

  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) {
    std::cerr << "Failed to open database connection" << std::endl;
    return chats;
  }

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      ChatInfo chat;
      int uuidLen = sqlite3_column_bytes(stmt, 0);
      int nameLen = sqlite3_column_bytes(stmt, 1);

      const unsigned char *uuidText = sqlite3_column_text(stmt, 0);
      const unsigned char *nameText = sqlite3_column_text(stmt, 1);

      chat.uuid = QString::fromUtf8(reinterpret_cast<const char *>(uuidText), uuidLen);
      chat.name = QString::fromUtf8(reinterpret_cast<const char *>(nameText), nameLen);

      const void *blob = sqlite3_column_blob(stmt, 2);
      int blobSize = sqlite3_column_bytes(stmt, 2);
      if (blob && blobSize > 0) {
        chat.avatar = QByteArray(static_cast<const char *>(blob), blobSize);
      }

      chats.append(chat);
    }
  }

  sqlite3_finalize(stmt);
  closeConnection(handle);
  return chats;
}

QList<DmChatDB::MessageInfo> DmChatDB::getChatMessages(const QString &chatUuid) {
  QList<MessageInfo> messages;
  const char *sql =
      "SELECT sender_uuid, content, timestamp, sender_name, sender_avatar, is_history "
      "FROM messages WHERE chat_uuid = ? ORDER BY timestamp ASC;";

  sqlite3 *handle = nullptr;
  if (!openConnection(handle)) return messages;

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    QByteArray chatUuidUtf8 = chatUuid.toUtf8();
    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      MessageInfo msg;
      const unsigned char *texts[4];
      int lengths[4];

      for (int i = 0; i < 4; i++) {
        texts[i] = sqlite3_column_text(stmt, i);
        lengths[i] = sqlite3_column_bytes(stmt, i);
      }

      msg.senderUuid = QString::fromUtf8(reinterpret_cast<const char *>(texts[0]), lengths[0]);
      msg.content = QString::fromUtf8(reinterpret_cast<const char *>(texts[1]), lengths[1]);
      msg.timestamp = QString::fromUtf8(reinterpret_cast<const char *>(texts[2]), lengths[2]);
      msg.senderName = QString::fromUtf8(reinterpret_cast<const char *>(texts[3]), lengths[3]);

      const void *blob = sqlite3_column_blob(stmt, 4);
      int blobSize = sqlite3_column_bytes(stmt, 4);
      if (blob && blobSize > 0) {
        msg.senderAvatar = QByteArray(static_cast<const char *>(blob), blobSize);
      }

      msg.isHistory = sqlite3_column_int(stmt, 5) != 0;
      messages.append(msg);
    }
  }

  sqlite3_finalize(stmt);
  closeConnection(handle);
  return messages;
}
