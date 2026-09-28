#pragma once

#include <format>
#include <string>

#include "util/ascii.hpp"
#include "util/symbol.hpp"

namespace formal {

[[nodiscard]] constexpr std::string dot_label(Symbol symbol) {
    if (symbol == '"' || symbol == '\\') {
        return std::string{'\\'} + to_char(symbol);
    }
    if (is_print(symbol)) { return std::string{to_char(symbol)}; }
    return std::format("\\\\x{:02x}", symbol);
}

} // namespace formal
