include(FetchContent)

# Keep the sources outside the build tree, so that removing it does not mean
# downloading them again. Only the sources are shared: every preset still
# builds googletest with its own flags.
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

add_executable(formal_tests regex/lexer_test.cpp regex/token_test.cpp)
target_link_libraries(
    formal_tests
    PRIVATE formal GTest::gtest_main formal_coverage formal_warnings
)

include(GoogleTest)
gtest_discover_tests(formal_tests)
