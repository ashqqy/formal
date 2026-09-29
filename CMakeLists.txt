cmake_minimum_required(VERSION 3.25)

project(formal LANGUAGES CXX)

set(FORMAL_MINIMUM_VERSION "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(FORMAL_MINIMUM_VERSION 14)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(FORMAL_MINIMUM_VERSION 18)
endif()

if(
    FORMAL_MINIMUM_VERSION STREQUAL ""
    OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS FORMAL_MINIMUM_VERSION
)
    message(
        FATAL_ERROR
        "GCC 14+ or Clang 18+ required, found ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.\n"
        "Delete the build directory and re-run with the compiler set, e.g.:\n"
        "  CXX=g++ cmake --workflow --preset release\n"
        "  CXX=clang++ cmake --workflow --preset release"
    )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)

file(
    CREATE_LINK
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    SYMBOLIC
)

include(cmake/coverage.cmake)
include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)

add_subdirectory(src)
add_subdirectory(examples)

# Reads file lists from the targets above.
include(cmake/tidy.cmake)

enable_testing()
add_subdirectory(tests)
