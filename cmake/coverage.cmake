option(FORMAL_COVERAGE "Build with coverage instrumentation" OFF)

add_library(formal_coverage INTERFACE)

if(NOT FORMAL_COVERAGE)
    return()
endif()

target_compile_options(formal_coverage INTERFACE --coverage)
target_link_options(formal_coverage INTERFACE --coverage)

find_program(GCOVR gcovr REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-search-dirs
        OUTPUT_VARIABLE GCC_SEARCH_DIRS
    )
    string(REGEX MATCH "install: ([^\n]*)" _ "${GCC_SEARCH_DIRS}")
    cmake_path(SET GCC_BIN NORMALIZE "${CMAKE_MATCH_1}/../../../../bin")

    find_program(GCOV gcov HINTS ${GCC_BIN} REQUIRED)
    set(GCOV_COMMAND ${GCOV})
else()
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH CLANG_BIN)

    find_program(LLVM_COV llvm-cov HINTS ${CLANG_BIN} REQUIRED)
    set(GCOV_COMMAND "${LLVM_COV} gcov")
endif()

add_custom_target(
    coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage
    COMMAND
        ${GCOVR} --root ${CMAKE_SOURCE_DIR} --filter ${CMAKE_SOURCE_DIR}/src/
        --gcov-executable ${GCOV_COMMAND} --html-details coverage/index.html
        --print-summary --delete
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report"
)
