add_library(formal INTERFACE)
target_include_directories(formal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(formal INTERFACE -freflection)
