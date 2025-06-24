cmake_minimum_required(VERSION 3.20)
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/Backend/VM-API)

add_custom_target(VM-API ALL
    # Ensure output directory exists
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    # Run go mod tidy in the module root
    COMMAND go mod tidy
    # Build the Go binary into the output directory
    COMMAND go build -o ${OUTPUT_DIR}/VM-API ${CMAKE_CURRENT_SOURCE_DIR}/src/main.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Go VM-API binary"
)