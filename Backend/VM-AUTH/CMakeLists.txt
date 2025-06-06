project(VM_API LANGUAGES CXX)

add_executable(VM-AUTH src/main.cpp)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend/VM-AUTH)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-AUTH Qt6::Core Qt6::WebSockets)