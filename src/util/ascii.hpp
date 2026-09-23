#pragma once

#include "util/symbol.hpp"

namespace formal {

[[nodiscard]] constexpr bool is_space(Symbol symbol) noexcept {
    return symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r';
}

[[nodiscard]] constexpr bool is_print(Symbol symbol) noexcept {
    return symbol >= 0x20 && symbol <= 0x7e;
}

} // namespace formal
