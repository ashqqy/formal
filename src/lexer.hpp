#pragma once

#include <string>
#include <utility>

#include "error.hpp" // IWYU pragma: export
#include "position.hpp"
#include "symbol.hpp"
#include "token.hpp"

namespace regex {

class Lexer {
  public:
    explicit Lexer(std::string input) : input_(std::move(input)) {}

    Token next();

  private:
    [[nodiscard]] bool eof() const noexcept {
        return cursor_.offset >= input_.size();
    }
    [[nodiscard]] Symbol peek() const noexcept {
        return static_cast<Symbol>(input_[cursor_.offset]);
    }
    Symbol advance() noexcept {
        Symbol symbol = peek();
        cursor_.advance(symbol);
        return symbol;
    }

    std::string input_;
    Position cursor_;
};

} // namespace regex
