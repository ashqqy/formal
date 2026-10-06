#pragma once

#include <ostream>
#include <string>
#include <utility>

#include "regex/error.hpp" // IWYU pragma: export
#include "regex/node.hpp"
#include "regex/parser.hpp"
#include "regex/printer.hpp"
#include "util/alphabet.hpp"

namespace formal {

class Regex {
  public:
    constexpr explicit Regex(std::string pattern,
                             Alphabet allowed = Alphabet::all())
        : root_(regex::parse(std::move(pattern), allowed)) {}

    [[nodiscard]] constexpr std::string to_string() const {
        return regex::to_string(*root_);
    }

  private:
    regex::NodePtr root_;
};

inline std::ostream& operator<<(std::ostream& os, const Regex& pattern) {
    return os << pattern.to_string();
}

} // namespace formal
