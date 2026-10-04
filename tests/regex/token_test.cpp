#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "regex/position.hpp"
#include "regex/token.hpp"
#include "regex/token_print.hpp"
#include "util/symbol.hpp"

namespace formal::regex {
namespace {

Position start() {
    return Position{.line = 1, .column = 1, .offset = 0};
}

std::string print(const Token& token) {
    std::ostringstream out;
    out << token;
    return out.str();
}

static_assert(name(TokenType::Pipe) == "Pipe");
static_assert(name(TokenType::Star) == "Star");
static_assert(name(TokenType::LParen) == "LParen");
static_assert(name(TokenType::RParen) == "RParen");
static_assert(name(TokenType::Letter) == "Letter");
static_assert(name(TokenType::End) == "End");
static_assert(name(static_cast<TokenType>(42)) == "<unknown>");

TEST(TokenTypeName, NamesEveryTokenType) {
    EXPECT_EQ(name(TokenType::Pipe), "Pipe");
    EXPECT_EQ(name(TokenType::Star), "Star");
    EXPECT_EQ(name(TokenType::LParen), "LParen");
    EXPECT_EQ(name(TokenType::RParen), "RParen");
    EXPECT_EQ(name(TokenType::Letter), "Letter");
    EXPECT_EQ(name(TokenType::End), "End");
}

TEST(TokenTypePrint, WritesTheName) {
    std::ostringstream out;
    out << TokenType::LParen;
    EXPECT_EQ(out.str(), "LParen");
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
