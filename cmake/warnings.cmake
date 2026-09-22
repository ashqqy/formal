option(FORMAL_WERROR "Treat warnings as errors" OFF)

add_library(formal_warnings INTERFACE)

target_compile_options(
    formal_warnings
    INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wnull-dereference
        -Wimplicit-fallthrough
)

if(FORMAL_WERROR)
    target_compile_options(formal_warnings INTERFACE -Werror)
endif()
