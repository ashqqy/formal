cmake_minimum_required(VERSION 3.25)

project(formal LANGUAGES CXX)

if(
    NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16
)
    message(
        FATAL_ERROR
        "GCC 16+ required, found ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.\n"
        "Delete the build directory and re-run with the compiler set, e.g.:\n"
        "  CXX=g++-16 cmake --workflow --preset release"
    )
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/coverage.cmake)
include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
