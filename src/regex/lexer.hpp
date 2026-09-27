#pragma once

#include <string>
#include <utility>

#include "regex/error.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/ascii.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

class Lexer {
  public:
    constexpr explicit Lexer(std::string input) noexcept
        : input_(std::move(input)) {}

    [[nodiscard]] constexpr Token next() { return scan(cursor_); }

    [[nodiscard]] constexpr Token peek() const {
        Position cursor = cursor_;
        return scan(cursor);
    }

  private:
    [[nodiscard]] constexpr Token scan(Position& cursor) const {
        while (!eof(cursor) && is_space(symbol_at(cursor))) {
            advance(cursor);
        }

        if (eof(cursor)) { return {TokenType::End, '\0', cursor}; }

        const Position start = cursor;
        const Symbol c = advance(cursor);

        switch (c) {
            case '|':
                return {TokenType::Pipe, c, start};
            case '*':
                return {TokenType::Star, c, start};
            case '(':
                return {TokenType::Lparen, c, start};
            case ')':
                return {TokenType::Rparen, c, start};
            case '\\': {
                if (eof(cursor)) {
                    throw SyntaxError("Dangling backslash", start);
                }
                return {TokenType::Letter, advance(cursor), start};
            }
            default:
                return {TokenType::Letter, c, start};
        }
    }

    [[nodiscard]] constexpr bool eof(const Position& cursor) const noexcept {
        return cursor.offset >= input_.size();
    }
    [[nodiscard]] constexpr Symbol
    symbol_at(const Position& cursor) const noexcept {
        return to_symbol(input_[cursor.offset]);
    }
    constexpr Symbol advance(Position& cursor) const noexcept {
        const Symbol symbol = symbol_at(cursor);
        cursor.advance(symbol);
        return symbol;
    }

    std::string input_;
    Position cursor_;
};

} // namespace formal::regex
