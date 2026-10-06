include(FetchContent)

# Sources outside the build tree; each preset still builds its own gtest.
set(GOOGLETEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/.deps/googletest)
if(EXISTS ${GOOGLETEST_SOURCE_DIR}/CMakeLists.txt)
    set(FETCHCONTENT_SOURCE_DIR_GOOGLETEST ${GOOGLETEST_SOURCE_DIR})
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    SOURCE_DIR ${GOOGLETEST_SOURCE_DIR}
)
FetchContent_MakeAvailable(googletest)

add_executable(
    formal_tests
    regex_test.cpp
    automata/closure_test.cpp
    automata/dot_test.cpp
    automata/nfa_test.cpp
    regex/dot_test.cpp
    regex/node_test.cpp
    regex/parser_test.cpp
    regex/printer_test.cpp
    regex/lexer_test.cpp
    regex/thompson_test.cpp
    regex/token_test.cpp
    util/alphabet_test.cpp
    util/sorted_set_test.cpp
)
target_link_libraries(
    formal_tests
    PRIVATE formal GTest::gtest_main formal_coverage formal_warnings
)

include(GoogleTest)
gtest_discover_tests(formal_tests)
