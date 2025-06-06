cmake_minimum_required(VERSION 3.20)
project(VentraMessenger)

set(BACKEND_SERVICES VM-API VM-AUTH VM-LOGIC VM-MAIN-DB VM-REDIS)

# Set Release as default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Subprojects
add_subdirectory(Backend)
add_subdirectory(QTClient)

# Frontend target: only build QTClient
add_custom_target(frontend
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR}/QTClient -B ${CMAKE_BINARY_DIR}/QTClient
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/QTClient
    COMMENT "Configuring and building frontend application"
)

# Backend target: configure, build, copy executables, build Docker images
add_custom_target(backend
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR}/Backend -B ${CMAKE_SOURCE_DIR}/cmake-build-backend
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/cmake-build-backend
    COMMENT "Configuring and building backend"
)

foreach(service ${BACKEND_SERVICES})
  add_custom_command(TARGET backend POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/build/Backend/${service}
      COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/cmake-build-backend/${service}/${service}
        ${CMAKE_SOURCE_DIR}/build/Backend/${service}/
  )
endforeach()

add_custom_command(TARGET backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Building Docker images and starting containers..."
    COMMAND docker compose -f ${CMAKE_SOURCE_DIR}/Backend/docker-compose.yml build
    COMMAND docker compose -f ${CMAKE_SOURCE_DIR}/Backend/docker-compose.yml up -d
)