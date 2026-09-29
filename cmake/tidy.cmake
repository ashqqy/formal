find_program(CLANG_TIDY clang-tidy)

if(NOT CLANG_TIDY)
    return()
endif()

get_target_property(FORMAL_HEADERS formal HEADER_SET)

add_custom_target(
    tidy
    COMMAND
        ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} --quiet ${FORMAL_HEADERS}
        ${CMAKE_SOURCE_DIR}/examples/print.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-tidy"
)
