cmake_minimum_required(VERSION 3.20)
project(Backend)

# Erzwinge Release-Build
set(CMAKE_BUILD_TYPE Release)

# Gemeinsames Build-Verzeichnis
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_custom_target(docker_compose_up
    COMMAND docker compose up --build -d
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building and starting Docker Compose services"
)

# Unterprojekte hinzufügen
add_subdirectory(VM-LOGIC)
add_subdirectory(VM-AUTH)
add_subdirectory(VM-API)
