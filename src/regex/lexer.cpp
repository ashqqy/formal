#include "regex/lexer.hpp"

#include <cctype>

#include "regex/error.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

Token Lexer::next() {
    while (!eof() && std::isspace(peek()) != 0) {
        advance();
    }

    if (eof()) { return {TokenType::End, '\0', cursor_}; }

    Position start = cursor_;
    Symbol c = advance();

    switch (c) {
        case '+':
            return {TokenType::Plus, c, start};
        case '.':
            return {TokenType::Dot, c, start};
        case '*':
            return {TokenType::Star, c, start};
        case '(':
            return {TokenType::Lparen, c, start};
        case ')':
            return {TokenType::Rparen, c, start};
        case '1':
            return {TokenType::Epsilon, c, start};
        case '\\': {
            if (eof()) { throw SyntaxError("Dangling backslash", start); }
            Symbol d = advance();
            return {TokenType::Letter, d, start};
        }
        default:
            return {TokenType::Letter, c, start};
    }
}

} // namespace formal::regex
