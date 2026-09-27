#pragma once

#include <format>
#include <ostream>

#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/ascii.hpp"
#include "util/enum_name.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    return os << enum_name(type);
}

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.type();
    os << " '";
    if (is_print(token.symbol())) {
        os << to_char(token.symbol());
    } else {
        os << std::format("\\x{:02x}", token.symbol());
    }
    os << '\'';
    return os << " at " << token.position();
}

} // namespace formal::regex
