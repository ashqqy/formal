#pragma once

#include <cstdint>
#include <format>
#include <ostream>

#include "regex/position.hpp"
#include "util/ascii.hpp"
#include "util/enum_name.hpp"
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

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    return os << enum_name(type);
}

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

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.type();
    os << " '";
    if (is_print(token.symbol())) {
        os << static_cast<char>(token.symbol());
    } else {
        os << std::format("\\x{:02x}", token.symbol());
    }
    os << '\'';
    return os << " at " << token.position();
}

} // namespace formal::regex
