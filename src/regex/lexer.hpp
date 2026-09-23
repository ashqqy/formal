#pragma once

#include <string>
#include <utility>

#include "regex/error.hpp" // IWYU pragma: export
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/ascii.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

class Lexer {
  public:
    constexpr explicit Lexer(std::string input) noexcept
        : input_(std::move(input)) {}

    [[nodiscard]] constexpr Token next() {
        while (!eof() && is_space(peek())) {
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

  private:
    [[nodiscard]] constexpr bool eof() const noexcept {
        return cursor_.offset >= input_.size();
    }
    [[nodiscard]] constexpr Symbol peek() const noexcept {
        return static_cast<Symbol>(input_[cursor_.offset]);
    }
    constexpr Symbol advance() noexcept {
        Symbol symbol = peek();
        cursor_.advance(symbol);
        return symbol;
    }

    std::string input_;
    Position cursor_;
};

} // namespace formal::regex
