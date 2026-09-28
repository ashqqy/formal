#pragma once

#include <format>
#include <ostream>
#include <string_view>

#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/ascii.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

[[nodiscard]] constexpr std::string_view name(TokenType type) {
    switch (type) {
        case TokenType::Pipe:
            return "Pipe";
        case TokenType::Star:
            return "Star";
        case TokenType::LParen:
            return "LParen";
        case TokenType::RParen:
            return "RParen";
        case TokenType::Letter:
            return "Letter";
        case TokenType::End:
            return "End";
    }
    return "<unknown>";
}

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    return os << name(type);
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
