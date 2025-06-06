cmake_minimum_required(VERSION 3.16)
project(Ventra-Messenger)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/QTClient)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/Release)

file(MAKE_DIRECTORY ${OUTPUT_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets WebSockets REQUIRED)

# OpenSSL configuration
set(OPENSSL_USE_STATIC_LIBS OFF)
set(OPENSSL_USE_SHARED_LIBS ON)
find_package(OpenSSL REQUIRED)

# SQLCipher über pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)

# Add executable
add_executable(Ventra-Messenger
    src/main.cpp
    res.qrc
    src/Crypto/Encryption/ChaCha20.cpp
    src/Crypto/Encryption/ChaCha20.h
    src/Crypto/Encryption/AES256.cpp
    src/Crypto/Encryption/AES256.h
    src/Crypto/Encryption/EncryptionEnv.h
    src/Crypto/Encryption/EncryptionEnv.cpp
    src/HelperUtils/HelperUtils.cpp
    src/HelperUtils/HelperUtils.h
    src/Crypto/Hash/BLAKE2b512.cpp
    src/Crypto/Hash/BLAKE2b512.h
    src/Crypto/Hash/HashingEnv.h
    src/Crypto/Hash/HashingEnv.cpp
    src/Crypto/Hash/BLAKE2s256.cpp
    src/Crypto/Hash/BLAKE2s256.h
    src/Crypto/KeyEnv/KeyEnv.cpp
    src/Crypto/KeyEnv/KeyEnv.h
    src/Crypto/KeyEnv/X25519KeyPair.cpp
    src/Crypto/KeyEnv/X25519KeyPair.h
    src/Crypto/KeyEnv/RandomVec.cpp
    src/Crypto/KeyEnv/RandomVec.h
    src/Crypto/DoubleRatchet/DoubleRatchet.h
    src/Crypto/DoubleRatchet/DoubleRatchet.cpp
    src/ThreadPool/ThreadPool.cpp
    src/ThreadPool/ThreadPool.tpp
    src/ThreadPool/ThreadPool.h
    src/ThreadPool/ThreadSaveVector.cpp
    src/ThreadPool/ThreadSaveVector.h
    src/Crypto/KDF/HKDF.cpp
    src/Crypto/KDF/HKDF.h
    src/Crypto/KDF/KDFEnv.cpp
    src/Crypto/KDF/KDFEnv.h
    src/Database/LocalDatabase.cpp
    src/Database/LocalDatabase.h
    src/Gui/Sidebar/Sidebar.cpp
    src/Gui/Sidebar/Sidebar.h
    src/Gui/MainWindow/MainWindow.cpp
    src/Gui/MainWindow/MainWindow.h
    src/Gui/GuiHelper/GuiHelper.cpp
    src/Gui/GuiHelper/GuiHelper.h
    src/Gui/ContactList/ContactList.cpp
    src/Gui/ContactList/ContactList.h
    src/Gui/ContactList/ContactButton.cpp
    src/Gui/ContactList/ContactButton.h
    src/Gui/ChatWindow/ChatWindow.cpp
    src/Gui/ChatWindow/ChatWindow.h
    src/Gui/ChatWindow/ChatInputBar.cpp
    src/Gui/ChatWindow/ChatInputBar.h
    src/Gui/ChatWindow/Message.cpp
    src/Gui/ChatWindow/Message.h
    src/Gui/ChatWindow/MessageTextWidget.cpp
    src/Gui/ChatWindow/MessageTextWidget.h
    src/Gui/DirektChatScreen/DirektChatScreen.cpp
    src/Gui/DirektChatScreen/DirektChatScreen.h
    src/Crypto/IDs/GenerateID.cpp
    src/Crypto/IDs/GenerateID.h
    src/Logic/GuiUpdates/DMChatGuiManager.cpp
    src/Logic/GuiUpdates/DMChatGuiManager.h
    src/Logic/DataBaseOperations/DMChatDBManager.cpp
    src/Logic/DataBaseOperations/DMChatDBManager.h
    src/Logic/ScreenManager/DMChatManager.cpp
    src/Logic/ScreenManager/DMChatManager.h
    src/Gui/Gui_Structs_Enums.h
    src/Gui/ContactList/ContactListSearch.cpp
    src/Gui/ContactList/ContactListSearch.h
    src/Logic/DataBaseOperations/UserDataDB.cpp
    src/Logic/DataBaseOperations/UserDataDB.h
    src/Logic/OnlineConnection/WebSocketClient.cpp
    src/Logic/OnlineConnection/WebSocketClient.h
)

# Include dirs for SQLCipher
target_link_directories(Ventra-Messenger PRIVATE ${SQLCIPHER_LIBRARY_DIRS})

# Link libraries
target_link_libraries(Ventra-Messenger
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
    sqlcipher
)

target_compile_definitions(Ventra-Messenger PRIVATE SQLITE_HAS_CODEC=1)

# Debug output
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "SQLCipher include dirs: ${SQLCIPHER_INCLUDE_DIRS}")
message(STATUS "SQLCipher libraries: ${SQLCIPHER_LIBRARIES}")
