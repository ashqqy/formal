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
    constexpr explicit Lexer(std::string input) noexcept
        : input_(std::move(input)) {}

    [[nodiscard]] Token next();

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

} // namespace regex
