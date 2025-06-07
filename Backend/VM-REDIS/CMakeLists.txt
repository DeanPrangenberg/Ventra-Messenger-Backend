cmake_minimum_required(VERSION 3.16)
project(VM_API LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_executable(VM-REDIS src/main.cpp)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-REDIS Qt6::Core Qt6::WebSockets)