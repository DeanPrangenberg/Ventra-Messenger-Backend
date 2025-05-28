//
// Created by deanprangenberg on 25.05.25.
//

#include "DMChatDBManager.h"

namespace Logic {
  DMChatDBManager::DMChatDBManager(const fs::path &dbPath, const std::string &password, bool debugMode)
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

  bool DMChatDBManager::createChatTables() {
    std::string createChatsTable =
        "CREATE TABLE IF NOT EXISTS chats ("
        "chat_uuid TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "avatar BLOB"
        ");";

    std::string createMessagesTable =
        "CREATE TABLE IF NOT EXISTS messages ("
        "message_id TEXT PRIMARY KEY,"
        "chat_uuid TEXT NOT NULL,"
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

  QList<Gui::chatData> DMChatDBManager::getAllChats() {
    QList<Gui::chatData> chats;
    const char *sqlChatData = "SELECT chat_uuid, name, avatar FROM chats;";

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return chats;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sqlChatData, -1, &stmt, nullptr) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        Gui::chatData chat;

        const unsigned char *uuidText = sqlite3_column_text(stmt, 0);
        const unsigned char *nameText = sqlite3_column_text(stmt, 1);
        const void *blob = sqlite3_column_blob(stmt, 2);
        int blobSize = sqlite3_column_bytes(stmt, 2);

        if (uuidText) chat.chatUUID = QString::fromUtf8(reinterpret_cast<const char *>(uuidText));
        if (nameText) chat.name = QString::fromUtf8(reinterpret_cast<const char *>(nameText));

        if (blob && blobSize > 0) {
          QByteArray avatarData(static_cast<const char *>(blob), blobSize);
          QPixmap pixmap;
          pixmap.loadFromData(avatarData, "PNG");
          chat.avatar = pixmap;
        } else {
          chat.avatar = QPixmap(); // leerer Fallback
        }

        chat.messageContainerList = getChatMessages(chat.chatUUID);
        chats.append(chat);
      }
    }

    sqlite3_finalize(stmt);
    closeConnection(handle);
    return chats;
  }

  bool DMChatDBManager::insertChat(const Gui::chatData &chat) {
    std::string sql = "INSERT OR REPLACE INTO chats (chat_uuid, name, avatar) VALUES (?, ?, ?);";

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    QByteArray avatarData;
    if (!chat.avatar.isNull()) {
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      chat.avatar.save(&buffer, "PNG");
      buffer.close();
    }

    QByteArray chatUuidUtf8 = chat.chatUUID.toUtf8();
    QByteArray nameUtf8 = chat.name.toUtf8();

    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, nameUtf8.constData(), nameUtf8.length(), SQLITE_TRANSIENT);

    if (!avatarData.isEmpty()) {
      sqlite3_bind_blob(stmt, 3, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt, 3);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    closeConnection(handle);

    insertMessages(chat.messageContainerList);

    return rc == SQLITE_DONE;
  }

  bool DMChatDBManager::insertChats(const QList<Gui::chatData> &chats) {
    if (chats.isEmpty()) return true;

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    std::string sql = "INSERT OR REPLACE INTO chats (chat_uuid, name, avatar) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    bool allSuccess = true;
    for (const auto &chat: chats) {
      QByteArray avatarData;
      if (!chat.avatar.isNull()) {
        QBuffer buffer(&avatarData);
        buffer.open(QIODevice::WriteOnly);
        chat.avatar.save(&buffer, "PNG");
        buffer.close();
      }

      QByteArray chatUuidUtf8 = chat.chatUUID.toUtf8();
      QByteArray nameUtf8 = chat.name.toUtf8();

      sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, nameUtf8.constData(), nameUtf8.length(), SQLITE_TRANSIENT);

      if (!avatarData.isEmpty()) {
        sqlite3_bind_blob(stmt, 3, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
      } else {
        sqlite3_bind_null(stmt, 3);
      }

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        allSuccess = false;
        break;
      }

      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);

    if (allSuccess) {
      sqlite3_exec(handle, "COMMIT", nullptr, nullptr, nullptr);
    } else {
      sqlite3_exec(handle, "ROLLBACK", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);

    for (const auto &chat: chats) {
      if (!insertMessages(chat.messageContainerList)) {
        return false;
      }
    }

    return true;
  }

  QList<Gui::MessageContainer> DMChatDBManager::getChatMessages(const QString &chatUuid) {
    QList<Gui::MessageContainer> messages;
    const char *sql =
        "SELECT message_id, sender_uuid, content, timestamp, sender_name, sender_avatar, is_history "
        "FROM messages WHERE chat_uuid = ? ORDER BY timestamp ASC;";

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return messages;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) == SQLITE_OK) {
      QByteArray chatUuidUtf8 = chatUuid.toUtf8();
      sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);

      while (sqlite3_step(stmt) == SQLITE_ROW) {
        Gui::MessageContainer msg;
        msg.chatUUID = chatUuid;
        msg.messageUUID = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
        msg.senderUUID = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        msg.message = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        msg.time = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
        msg.senderName = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));

        const void *blob = sqlite3_column_blob(stmt, 5);
        int blobSize = sqlite3_column_bytes(stmt, 5);
        if (blob && blobSize > 0) {
          QByteArray avatarData(static_cast<const char *>(blob), blobSize);
          QPixmap pixmap;
          pixmap.loadFromData(avatarData);
          msg.avatar = pixmap;
        }

        msg.isFollowUp = sqlite3_column_int(stmt, 6) != 0;
        messages.append(msg);
      }
    }

    sqlite3_finalize(stmt);
    closeConnection(handle);
    return messages;
  }

  bool DMChatDBManager::insertMessages(const QList<Gui::MessageContainer> &messages) {
    if (messages.isEmpty()) return true;

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    std::string sql =
        "INSERT INTO messages "
        "(message_id, chat_uuid, sender_uuid, content, timestamp, sender_name, sender_avatar, is_history) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    bool success = true;
    for (const auto &msg: messages) {
      QByteArray avatarData;
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      msg.avatar.save(&buffer, "PNG");
      buffer.close();

      QByteArray messageIdUtf8 = msg.messageUUID.toUtf8();
      QByteArray chatUuidUtf8 = msg.chatUUID.toUtf8();
      QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
      QByteArray contentUtf8 = msg.message.toUtf8();
      QByteArray timestampUtf8 = msg.time.toUtf8();
      QByteArray senderNameUtf8 = msg.senderName.toUtf8();

      sqlite3_bind_text(stmt, 1, messageIdUtf8.constData(), messageIdUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, senderUuidUtf8.constData(), senderUuidUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, contentUtf8.constData(), contentUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, timestampUtf8.constData(), timestampUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 6, senderNameUtf8.constData(), senderNameUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_blob(stmt, 7, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 8, msg.isFollowUp ? 1 : 0);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
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

  bool DMChatDBManager::insertMessage(const Gui::MessageContainer &msg) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    std::string sql =
        "INSERT INTO messages "
        "(message_id, chat_uuid, sender_uuid, content, timestamp, sender_name, sender_avatar, is_history) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    bool success = false;
    if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      QByteArray avatarData;
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      msg.avatar.save(&buffer, "PNG");
      buffer.close();

      QByteArray messageIdUtf8 = msg.messageUUID.toUtf8();
      QByteArray chatUuidUtf8 = msg.chatUUID.toUtf8();
      QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
      QByteArray contentUtf8 = msg.message.toUtf8();
      QByteArray timestampUtf8 = msg.time.toUtf8();
      QByteArray senderNameUtf8 = msg.senderName.toUtf8();

      sqlite3_bind_text(stmt, 1, messageIdUtf8.constData(), messageIdUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, chatUuidUtf8.constData(), chatUuidUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, senderUuidUtf8.constData(), senderUuidUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, contentUtf8.constData(), contentUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, timestampUtf8.constData(), timestampUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 6, senderNameUtf8.constData(), senderNameUtf8.length(), SQLITE_TRANSIENT);
      sqlite3_bind_blob(stmt, 7, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 8, msg.isFollowUp ? 1 : 0);

      success = (sqlite3_step(stmt) == SQLITE_DONE);
    }

    sqlite3_finalize(stmt);
    closeConnection(handle);
    return success;
  }

  bool DMChatDBManager::deleteChat(const QString &chatUUID) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    const char *sqlDeleteMessages = "DELETE FROM messages WHERE chat_uuid = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sqlDeleteMessages, -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }
    sqlite3_bind_text(stmt, 1, chatUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }
    sqlite3_finalize(stmt);

    const char *sqlDeleteChat = "DELETE FROM chats WHERE chat_uuid = ?;";
    if (sqlite3_prepare_v2(handle, sqlDeleteChat, -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }
    sqlite3_bind_text(stmt, 1, chatUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }
    sqlite3_finalize(stmt);

    if (sqlite3_exec(handle, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);
    return true;
  }

  bool DMChatDBManager::deleteChats(const QList<QString> &chatUUIDs) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    const char *sqlDeleteMessages = "DELETE FROM messages WHERE chat_uuid = ?;";
    const char *sqlDeleteChat = "DELETE FROM chats WHERE chat_uuid = ?;";
    sqlite3_stmt *stmtMessages;
    sqlite3_stmt *stmtChats;

    if (sqlite3_prepare_v2(handle, sqlDeleteMessages, -1, &stmtMessages, nullptr) != SQLITE_OK ||
        sqlite3_prepare_v2(handle, sqlDeleteChat, -1, &stmtChats, nullptr) != SQLITE_OK) {
      if (stmtMessages) sqlite3_finalize(stmtMessages);
      if (stmtChats) sqlite3_finalize(stmtChats);
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    for (const auto &chatUUID: chatUUIDs) {
      sqlite3_reset(stmtMessages);
      sqlite3_bind_text(stmtMessages, 1, chatUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);
      if (sqlite3_step(stmtMessages) != SQLITE_DONE) {
        sqlite3_finalize(stmtMessages);
        sqlite3_finalize(stmtChats);
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }
      sqlite3_reset(stmtChats);
      sqlite3_bind_text(stmtChats, 1, chatUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);
      if (sqlite3_step(stmtChats) != SQLITE_DONE) {
        sqlite3_finalize(stmtMessages);
        sqlite3_finalize(stmtChats);
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }
    }

    sqlite3_finalize(stmtMessages);
    sqlite3_finalize(stmtChats);

    if (sqlite3_exec(handle, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);
    return true;
  }

  bool DMChatDBManager::deleteMessage(const QString &messageUUID) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    const char *sql = "DELETE FROM messages WHERE message_id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    sqlite3_bind_text(stmt, 1, messageUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    closeConnection(handle);
    return success;
  }

  bool DMChatDBManager::deleteMessages(const QList<QString> &messageUUIDs) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    for (const auto &messageUUID: messageUUIDs) {
      const char *sql = "DELETE FROM messages WHERE message_id = ?;";
      sqlite3_stmt *stmt;
      if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }

      sqlite3_bind_text(stmt, 1, messageUUID.toUtf8().constData(), -1, SQLITE_TRANSIENT);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }

      sqlite3_finalize(stmt);
    }

    if (sqlite3_exec(handle, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);
    return true;
  }

  bool DMChatDBManager::updateChat(const Gui::chatData &chat) {
    std::string sql = "UPDATE chats SET name = ?, avatar = ? WHERE chat_uuid = ?;";

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    QByteArray nameUtf8 = chat.name.toUtf8();
    QByteArray chatUuidUtf8 = chat.chatUUID.toUtf8();

    QByteArray avatarData;
    if (!chat.avatar.isNull()) {
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      chat.avatar.save(&buffer, "PNG");
      buffer.close();
    }

    sqlite3_bind_text(stmt, 1, nameUtf8.constData(), nameUtf8.size(), SQLITE_TRANSIENT);
    if (!avatarData.isEmpty()) {
      sqlite3_bind_blob(stmt, 2, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt, 2);
    }
    sqlite3_bind_text(stmt, 3, chatUuidUtf8.constData(), chatUuidUtf8.size(), SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    sqlite3_finalize(stmt);
    closeConnection(handle);

    return success;
  }

  bool DMChatDBManager::updateChats(const QList<Gui::chatData> &chats) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    const char *sql = "UPDATE chats SET name = ?, avatar = ? WHERE chat_uuid = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    for (const auto &chat: chats) {
      sqlite3_reset(stmt);

      QByteArray nameUtf8 = chat.name.toUtf8();
      QByteArray chatUuidUtf8 = chat.chatUUID.toUtf8();

      QByteArray avatarData;
      if (!chat.avatar.isNull()) {
        QBuffer buffer(&avatarData);
        buffer.open(QIODevice::WriteOnly);
        chat.avatar.save(&buffer, "PNG");
        buffer.close();
      }

      sqlite3_bind_text(stmt, 1, nameUtf8.constData(), nameUtf8.size(), SQLITE_TRANSIENT);
      if (!avatarData.isEmpty()) {
        sqlite3_bind_blob(stmt, 2, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
      } else {
        sqlite3_bind_null(stmt, 2);
      }
      sqlite3_bind_text(stmt, 3, chatUuidUtf8.constData(), chatUuidUtf8.size(), SQLITE_TRANSIENT);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }
    }

    sqlite3_finalize(stmt);
    if (sqlite3_exec(handle, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);
    return true;
  }

  bool DMChatDBManager::updateMessage(const Gui::MessageContainer &msg) {
    std::string sql =
        "UPDATE messages SET "
        "chat_uuid = ?, sender_uuid = ?, content = ?, timestamp = ?, "
        "sender_name = ?, sender_avatar = ?, is_history = ? "
        "WHERE message_id = ?;";

    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    QByteArray chatUuidUtf8 = msg.chatUUID.toUtf8();
    QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
    QByteArray contentUtf8 = msg.message.toUtf8();
    QByteArray timestampUtf8 = msg.time.toUtf8();
    QByteArray senderNameUtf8 = msg.senderName.toUtf8();
    QByteArray messageIdUtf8 = msg.messageUUID.toUtf8();

    QByteArray avatarData;
    if (!msg.avatar.isNull()) {
      QBuffer buffer(&avatarData);
      buffer.open(QIODevice::WriteOnly);
      msg.avatar.save(&buffer, "PNG");
      buffer.close();
    }

    sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, senderUuidUtf8.constData(), senderUuidUtf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contentUtf8.constData(), contentUtf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestampUtf8.constData(), timestampUtf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, senderNameUtf8.constData(), senderNameUtf8.size(), SQLITE_TRANSIENT);
    if (!avatarData.isEmpty()) {
      sqlite3_bind_blob(stmt, 6, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt, 6);
    }
    sqlite3_bind_int(stmt, 7, msg.isFollowUp ? 1 : 0);
    sqlite3_bind_text(stmt, 8, messageIdUtf8.constData(), messageIdUtf8.size(), SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    sqlite3_finalize(stmt);
    closeConnection(handle);

    return success;
  }

  bool DMChatDBManager::updateMessages(const QList<Gui::MessageContainer> &messages) {
    sqlite3 *handle = nullptr;
    if (!openConnection(handle)) return false;

    if (sqlite3_exec(handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    const char *sql =
        "UPDATE messages SET "
        "chat_uuid = ?, sender_uuid = ?, content = ?, timestamp = ?, "
        "sender_name = ?, sender_avatar = ?, is_history = ? "
        "WHERE message_id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(handle, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      closeConnection(handle);
      return false;
    }

    for (const auto &msg: messages) {
      sqlite3_reset(stmt);

      QByteArray chatUuidUtf8 = msg.chatUUID.toUtf8();
      QByteArray senderUuidUtf8 = msg.senderUUID.toUtf8();
      QByteArray contentUtf8 = msg.message.toUtf8();
      QByteArray timestampUtf8 = msg.time.toUtf8();
      QByteArray senderNameUtf8 = msg.senderName.toUtf8();
      QByteArray messageIdUtf8 = msg.messageUUID.toUtf8();

      QByteArray avatarData;
      if (!msg.avatar.isNull()) {
        QBuffer buffer(&avatarData);
        buffer.open(QIODevice::WriteOnly);
        msg.avatar.save(&buffer, "PNG");
        buffer.close();
      }

      sqlite3_bind_text(stmt, 1, chatUuidUtf8.constData(), chatUuidUtf8.size(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, senderUuidUtf8.constData(), senderUuidUtf8.size(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, contentUtf8.constData(), contentUtf8.size(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, timestampUtf8.constData(), timestampUtf8.size(), SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, senderNameUtf8.constData(), senderNameUtf8.size(), SQLITE_TRANSIENT);

      if (!avatarData.isEmpty()) {
        sqlite3_bind_blob(stmt, 6, avatarData.constData(), avatarData.size(), SQLITE_TRANSIENT);
      } else {
        sqlite3_bind_null(stmt, 6);
      }

      sqlite3_bind_int(stmt, 7, msg.isFollowUp ? 1 : 0);
      sqlite3_bind_text(stmt, 8, messageIdUtf8.constData(), messageIdUtf8.size(), SQLITE_TRANSIENT);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
        closeConnection(handle);
        return false;
      }
    }

    sqlite3_finalize(stmt);
    if (sqlite3_exec(handle, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
      sqlite3_exec(handle, "ROLLBACK;", nullptr, nullptr, nullptr);
      closeConnection(handle);
      return false;
    }

    closeConnection(handle);
    return true;
  }
} // Logic
