cmake_minimum_required(VERSION 3.20)
project(Backend)

# Erzwinge Release-Build
set(CMAKE_BUILD_TYPE Release)

# Gemeinsames Build-Verzeichnis
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Unterprojekte hinzufügen
add_subdirectory(VM-API)
add_subdirectory(VM-LOGIC)
add_subdirectory(VM-AUTH)
add_subdirectory(VM-MAIN-DB)
add_subdirectory(VM-REDIS)
