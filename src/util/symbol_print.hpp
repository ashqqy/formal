#pragma once

#include <format>
#include <string>

#include "util/ascii.hpp"
#include "util/symbol.hpp"

namespace formal {

[[nodiscard]] inline std::string escaped(Symbol symbol) {
    if (is_print(symbol)) { return std::string{to_char(symbol)}; }
    return std::format("\\x{:02x}", symbol);
}

} // namespace formal
