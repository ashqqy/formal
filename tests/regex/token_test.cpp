#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/enum_name.hpp"
#include "util/symbol.hpp"

namespace formal::regex {
namespace {

static_assert(enum_name(TokenType::Plus) == "Plus");
static_assert(enum_name(TokenType::Star) == "Star");
static_assert(enum_name(TokenType::End) == "End");
static_assert(enum_name(static_cast<TokenType>(42)) == "<unknown>");

Position start() {
    return Position{.line = 1, .column = 1, .offset = 0};
}

std::string print(const Token& token) {
    std::ostringstream out;
    out << token;
    return out.str();
}

TEST(TokenTypePrint, UsesEnumName) {
    std::ostringstream out;
    out << TokenType::Lparen;
    EXPECT_EQ(out.str(), "Lparen");
}

TEST(TokenTypePrint, UnknownValueHasAPlaceholder) {
    std::ostringstream out;
    out << static_cast<TokenType>(42);
    EXPECT_EQ(out.str(), "<unknown>");
}

TEST(TokenPrint, LetterShowsSymbolAndPosition) {
    const Token token{TokenType::Letter, 'a',
                      Position{.line = 2, .column = 3, .offset = 5}};
    EXPECT_EQ(print(token), "Letter 'a' at 2:3 (offset 5)");
}

TEST(TokenPrint, OperatorShowsItsSymbol) {
    const Token token{TokenType::Star, '*', start()};
    EXPECT_EQ(print(token), "Star '*' at 1:1 (offset 0)");
}

TEST(TokenPrint, NonPrintableSymbolIsHexEscaped) {
    const Token token{TokenType::Letter, Symbol{1}, start()};
    EXPECT_EQ(print(token), "Letter '\\x01' at 1:1 (offset 0)");
}

TEST(TokenPrint, HighByteIsNotSignExtended) {
    const Token token{TokenType::Letter, Symbol{0xff}, start()};
    EXPECT_EQ(print(token), "Letter '\\xff' at 1:1 (offset 0)");
}

TEST(TokenPrint, EndShowsItsNulSymbol) {
    const Token token{TokenType::End, '\0', start()};
    EXPECT_EQ(print(token), "End '\\x00' at 1:1 (offset 0)");
}

TEST(TokenPrint, GoogleTestUsesTheSameOutput) {
    const Token token{TokenType::Letter, 'z', start()};
    EXPECT_EQ(testing::PrintToString(token), print(token));
}

} // namespace
} // namespace formal::regex
