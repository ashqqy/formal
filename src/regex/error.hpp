#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

#include "regex/position.hpp"

namespace formal::regex {

class SyntaxError : public std::runtime_error {
  public:
    SyntaxError(std::string_view message, Position position)
        : std::runtime_error(std::string(message)), position_(position) {}

    [[nodiscard]] constexpr Position position() const noexcept {
        return position_;
    }

  private:
    Position position_;
};

} // namespace formal::regex
