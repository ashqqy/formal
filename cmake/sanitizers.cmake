option(FORMAL_SANITIZE "Build with sanitizers" OFF)

if(NOT FORMAL_SANITIZE)
    return()
endif()

add_compile_options(
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    -fno-sanitize-recover=all
)
add_link_options(-fsanitize=address,undefined)
