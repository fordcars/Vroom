cmake_minimum_required (VERSION 3.15)
project(VROOM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Optional, but useful if you use clangd

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(RESOURCE_DIR_NAME rsrc)
set(RESOURCE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_DIR_NAME})
if(WIN32)
    # Add Debug/ or Release/ subdirectory based on build type
    set(RESOURCE_DEST_DIR
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${RESOURCE_DIR_NAME})
else()
    set(RESOURCE_DEST_DIR
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RESOURCE_DIR_NAME})
endif()

add_subdirectory(src)

# Find clang-format
find_program(CLANG_FORMAT_EXECUTABLE
    NAMES clang-format clang-format-18 clang-format-17 clang-format-16 clang-format-15
    DOC "Path to clang-format executable"
)

if(CLANG_FORMAT_EXECUTABLE)
    # Collect all source files
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    )

    # clang-format-check target
    add_custom_target(clang-format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        --dry-run
        --Werror
        --style=file
        ${ALL_CXX_SOURCE_FILES}
        COMMENT "Checking code formatting with clang-format"
        VERBATIM
    )

    # clang-format-fix target
    add_custom_target(clang-format-fix
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -i
        --style=file
        ${ALL_CXX_SOURCE_FILES}
        COMMENT "Fixing code formatting with clang-format"
        VERBATIM
    )

    message(STATUS "clang-format found: ${CLANG_FORMAT_EXECUTABLE}")
    message(STATUS "Added targets: clang-format-check, clang-format-fix")
else()
    message(WARNING "clang-format not found. Formatting targets will not be available.")
endif()
