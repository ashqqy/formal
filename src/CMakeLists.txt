add_library(formal INTERFACE)
target_include_directories(formal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(
    formal
    INTERFACE
        FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
            FILES
                automata/closure.hpp
                automata/dot.hpp
                automata/nfa.hpp
                automata/state.hpp
                regex.hpp
                regex/dot.hpp
                regex/error.hpp
                regex/lexer.hpp
                regex/node.hpp
                regex/parser.hpp
                regex/position.hpp
                regex/printer.hpp
                regex/thompson.hpp
                regex/token.hpp
                regex/token_print.hpp
                regex/visitor.hpp
                util/alphabet.hpp
                util/ascii.hpp
                util/dot.hpp
                util/sorted_set.hpp
                util/symbol.hpp
                util/symbol_print.hpp
)
