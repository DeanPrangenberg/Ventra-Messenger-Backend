cmake_minimum_required(VERSION 3.16)
project(CryptGuard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set base output directory
set(BASE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../../build/QTClient)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BASE_OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BASE_OUTPUT_DIR}/Release)

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# OpenSSL configuration
set(OPENSSL_USE_STATIC_LIBS OFF)
set(OPENSSL_USE_SHARED_LIBS ON)
find_package(OpenSSL REQUIRED)

# Add executable
add_executable(CryptGuard
    src/main.cpp
    src/Crypto/Sync/ChaCha20.cpp
    src/Crypto/Sync/ChaCha20.h
    src/Crypto/Sync/AES256.cpp
    src/Crypto/Sync/AES256.h
    src/Crypto/Sync/EncryptionEnv.h
    src/Crypto/Sync/EncryptionEnv.cpp
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
    "src/Double Ratchet/DoubleRatchet.h"
    "src/Double Ratchet/DoubleRatchet.cpp"
    src/ThreadPool/ThreadPool.cpp
    src/ThreadPool/ThreadPool.tpp
    src/ThreadPool/ThreadPool.h
    src/ThreadPool/ThreadSaveVector.cpp
    src/ThreadPool/ThreadSaveVector.h
)

# Link libraries
target_link_libraries(CryptGuard
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Debug output
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL SSL lib: ${OPENSSL_SSL_LIBRARY}")
message(STATUS "OpenSSL crypto lib: ${OPENSSL_CRYPTO_LIBRARY}")