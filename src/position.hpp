#pragma once

#include <cstddef>
#include <ostream>

#include "symbol.hpp"

namespace regex {

struct Position {
    std::size_t line = 1;
    std::size_t column = 1;
    std::size_t offset = 0;

    bool operator==(const Position&) const = default;

    void advance(Symbol c) noexcept {
        ++offset;
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const Position& p) {
    return os << p.line << ":" << p.column << " (offset " << p.offset << ')';
}

} // namespace regex
