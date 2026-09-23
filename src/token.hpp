#pragma once

#include <cctype>
#include <cstdint>
#include <format>
#include <ostream>

#include "enum_name.hpp"
#include "position.hpp"
#include "symbol.hpp"

namespace regex {

enum class TokenType : std::uint8_t {
    Plus,
    Dot,
    Star,
    Lparen,
    Rparen,
    Epsilon,
    Letter,
    End,
};

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    return os << enum_name(type);
}

class Token {
  public:
    Token(TokenType type, Symbol symbol, Position position)
        : type_(type), symbol_(symbol), position_(position) {}

    bool operator==(const Token&) const = default;

    [[nodiscard]] TokenType type() const noexcept { return type_; }
    [[nodiscard]] Symbol symbol() const noexcept { return symbol_; }
    [[nodiscard]] Position position() const noexcept { return position_; }

  private:
    TokenType type_;
    Symbol symbol_;
    Position position_;
};

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.type();
    os << " '";
    if (std::isprint(token.symbol()) != 0) {
        os << token.symbol();
    } else {
        os << std::format("\\x{:02x}", token.symbol());
    }
    os << '\'';
    return os << " at " << token.position();
}

} // namespace regex
