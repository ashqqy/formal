#pragma once

#include <cstdint>

#include "regex/position.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

enum class TokenType : std::uint8_t {
    Pipe,
    Star,
    Lparen,
    Rparen,
    Letter,
    End,
};

class Token {
  public:
    constexpr Token(TokenType type, Symbol symbol, Position position) noexcept
        : type_(type), symbol_(symbol), position_(position) {}

    bool operator==(const Token&) const = default;

    [[nodiscard]] constexpr TokenType type() const noexcept { return type_; }
    [[nodiscard]] constexpr Symbol symbol() const noexcept { return symbol_; }
    [[nodiscard]] constexpr Position position() const noexcept {
        return position_;
    }

  private:
    TokenType type_;
    Symbol symbol_;
    Position position_;
};

} // namespace formal::regex
