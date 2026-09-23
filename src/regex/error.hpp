#pragma once

#include <exception>

#include "regex/position.hpp"

namespace formal::regex {

class SyntaxError : public std::exception {
  public:
    constexpr SyntaxError(const char* message, Position position) noexcept
        : message_(message), position_(position) {}

    [[nodiscard]] const char* what() const noexcept override {
        return message_;
    }

    [[nodiscard]] constexpr Position position() const noexcept {
        return position_;
    }

  private:
    const char* message_;
    Position position_;
};

} // namespace formal::regex
