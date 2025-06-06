project(VM_API LANGUAGES CXX)

add_executable(VM-API src/main.cpp)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/backend/VM-API)

target_link_options(VM-API PRIVATE -static-libstdc++ -static-libgcc)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)
target_link_libraries(VM-API Qt6::Core Qt6::WebSockets)