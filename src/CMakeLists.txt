add_library(formal regex/lexer.cpp)
target_include_directories(formal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(formal PUBLIC -freflection)
target_link_libraries(formal PRIVATE formal_coverage formal_warnings)
