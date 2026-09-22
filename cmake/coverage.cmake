option(FORMAL_COVERAGE "Build with coverage instrumentation" OFF)

add_library(formal_coverage INTERFACE)

if(NOT FORMAL_COVERAGE)
    return()
endif()

target_compile_options(formal_coverage INTERFACE --coverage)
target_link_options(formal_coverage INTERFACE --coverage)

find_program(GCOVR gcovr REQUIRED)

add_custom_target(
    coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage
    COMMAND
        ${GCOVR} --root ${CMAKE_SOURCE_DIR} --filter ${CMAKE_SOURCE_DIR}/src/
        --html-details coverage/index.html --print-summary --delete
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report"
)
