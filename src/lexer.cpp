#include "lexer.hpp"

#include <cctype>

#include "error.hpp"
#include "position.hpp"

namespace regex {

Token Lexer::next() {
    while (!eof() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
        advance();
    }

    if (eof()) { return {TokenType::End, '\0', cursor_}; }

    Position start = cursor_;
    char c = advance();

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
            char d = advance();
            return {TokenType::Letter, d, start};
        }
        default:
            return {TokenType::Letter, c, start};
    }
}

} // namespace regex
