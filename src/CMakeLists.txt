add_library(formal lexer.cpp)
target_include_directories(formal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(formal PUBLIC -freflection)
