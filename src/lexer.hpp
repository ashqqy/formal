#pragma once

#include <cctype>
#include <cstdint>
#include <string>
#include <utility>

#include "error.hpp"
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

    Token next() {
        while (!eof() &&
               std::isspace(static_cast<unsigned char>(peek())) != 0) {
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
