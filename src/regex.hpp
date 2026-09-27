#pragma once

#include <ostream>
#include <string>
#include <utility>

#include "regex/dot.hpp"
#include "regex/error.hpp" // IWYU pragma: export
#include "regex/node.hpp"
#include "regex/parser.hpp"
#include "regex/printer.hpp"

namespace formal {

class Regex {
  public:
    constexpr explicit Regex(std::string pattern)
        : root_(regex::parse(std::move(pattern))) {}

    [[nodiscard]] constexpr std::string to_string() const {
        regex::Printer printer;
        root_->accept(printer);
        return std::move(printer).take();
    }

    [[nodiscard]] constexpr std::string to_dot() const {
        regex::DotPrinter printer;
        root_->accept(printer);
        return std::move(printer).take();
    }

  private:
    regex::NodePtr root_;
};

inline std::ostream& operator<<(std::ostream& os, const Regex& pattern) {
    return os << pattern.to_string();
}

} // namespace formal
