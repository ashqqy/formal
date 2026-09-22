#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "error.hpp" // IWYU pragma: export
#include "position.hpp"

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

class Token {
  public:
    Token(TokenType type, char symbol, Position position)
        : type_(type), symbol_(symbol), position_(position) {}

    bool operator==(const Token&) const = default;

    [[nodiscard]] TokenType type() const noexcept { return type_; }
    [[nodiscard]] char symbol() const noexcept { return symbol_; }
    [[nodiscard]] Position position() const noexcept { return position_; }

  private:
    TokenType type_;
    char symbol_;
    Position position_;
};

class Lexer {
  public:
    explicit Lexer(std::string input) : input_(std::move(input)) {}

    Token next();

  private:
    [[nodiscard]] bool eof() const noexcept {
        return cursor_.offset >= input_.size();
    }
    [[nodiscard]] char peek() const noexcept { return input_[cursor_.offset]; }
    char advance() noexcept {
        char symbol = peek();
        cursor_.advance(symbol);
        return symbol;
    }

    std::string input_;
    Position cursor_;
};

} // namespace regex
