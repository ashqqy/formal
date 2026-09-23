#pragma once

#include <string>
#include <utility>

#include "regex/error.hpp" // IWYU pragma: export
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

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

} // namespace formal::regex
