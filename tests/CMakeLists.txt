include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

add_executable(formal_tests lexer_test.cpp)
target_link_libraries(
    formal_tests
    PRIVATE formal GTest::gtest_main formal_coverage formal_warnings
)

include(GoogleTest)
gtest_discover_tests(formal_tests)
