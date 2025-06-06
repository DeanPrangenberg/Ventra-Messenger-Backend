project(VM_API LANGUAGES CXX)

add_executable(VM-REDIS src/main.cpp)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend/VM-REDIS)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-REDIS Qt6::Core Qt6::WebSockets)