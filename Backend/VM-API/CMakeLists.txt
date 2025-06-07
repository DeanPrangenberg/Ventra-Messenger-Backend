cmake_minimum_required(VERSION 3.16)
project(VM_API LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_executable(VM-API
    src/main.cpp
    ../../Shared/Network/WebSocketClient.cpp
    ../../Shared/Network/WebSocketClient.h
    ../../Shared/Network/WebSocketServer.cpp
    ../../Shared/Network/WebSocketServer.h
)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-API Qt6::Core Qt6::WebSockets)