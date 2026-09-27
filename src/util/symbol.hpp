#pragma once

namespace formal {

using Symbol = unsigned char;

[[nodiscard]] constexpr Symbol to_symbol(char value) noexcept {
    return static_cast<Symbol>(value);
}

[[nodiscard]] constexpr char to_char(Symbol symbol) noexcept {
    return static_cast<char>(symbol);
}

} // namespace formal
