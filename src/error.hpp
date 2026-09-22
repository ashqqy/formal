#pragma once

#include <stdexcept>
#include <string>

#include "position.hpp"

namespace regex {

class SyntaxError : public std::runtime_error {
  public:
    SyntaxError(const std::string& message, Position position)
        : std::runtime_error(message), position_(position) {}

    [[nodiscard]] Position position() const noexcept { return position_; }

  private:
    Position position_;
};

} // namespace regex
