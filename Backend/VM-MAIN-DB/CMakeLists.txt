project(VM_API LANGUAGES CXX)

add_executable(VM-MAIN-DB src/main.cpp)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend/VM-MAIN-DB)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-MAIN-DB Qt6::Core Qt6::WebSockets)