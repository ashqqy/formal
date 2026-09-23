#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "regex/error.hpp"
#include "regex/lexer.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/enum_name.hpp"
#include "util/symbol.hpp"

namespace formal::regex {
namespace {

// Collects every token, End included, so that a test can compare whole inputs.
std::vector<Token> tokenize(std::string input) {
    Lexer lexer(std::move(input));
    std::vector<Token> tokens;
    for (;;) {
        Token token = lexer.next();
        tokens.push_back(token);
        if (token.type() == TokenType::End) { return tokens; }
    }
}

Position at(std::size_t line, std::size_t column, std::size_t offset) {
    return Position{.line = line, .column = column, .offset = offset};
}

TEST(Lexer, EmptyInputIsEndOnly) {
    const std::vector<Token> tokens = tokenize("");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0], Token(TokenType::End, '\0', at(1, 1, 0)));
}

TEST(Lexer, LettersKeepTheirSymbolsAndPositions) {
    const std::vector<Token> tokens = tokenize("ab");
    ASSERT_EQ(tokens.size(), 3U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, 'a', at(1, 1, 0)));
    EXPECT_EQ(tokens[1], Token(TokenType::Letter, 'b', at(1, 2, 1)));
    EXPECT_EQ(tokens[2].type(), TokenType::End);
}

TEST(Lexer, DigitOtherThanOneIsALetter) {
    const std::vector<Token> tokens = tokenize("0");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '0', at(1, 1, 0)));
}

struct OperatorCase {
    char input;
    TokenType type;
};

class LexerOperator : public testing::TestWithParam<OperatorCase> {};

TEST_P(LexerOperator, HasItsOwnType) {
    const OperatorCase operator_case = GetParam();
    const std::vector<Token> tokens =
        tokenize(std::string(1, operator_case.input));
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0],
              Token(operator_case.type,
                    static_cast<Symbol>(operator_case.input), at(1, 1, 0)));
}

INSTANTIATE_TEST_SUITE_P(
    Operators, LexerOperator,
    testing::Values(OperatorCase{'+', TokenType::Plus},
                    OperatorCase{'.', TokenType::Dot},
                    OperatorCase{'*', TokenType::Star},
                    OperatorCase{'(', TokenType::Lparen},
                    OperatorCase{')', TokenType::Rparen},
                    OperatorCase{'1', TokenType::Epsilon}),
    [](const testing::TestParamInfo<OperatorCase>& test_case) {
        return std::string(enum_name(test_case.param.type));
    });

TEST(LexerEscape, TurnsAnOperatorIntoALetter) {
    const std::vector<Token> tokens = tokenize("\\*");
    ASSERT_EQ(tokens.size(), 2U);
    // The position points at the backslash, not at the escaped symbol.
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '*', at(1, 1, 0)));
}

TEST(LexerEscape, TurnsEpsilonIntoALetter) {
    const std::vector<Token> tokens = tokenize("\\1");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '1', at(1, 1, 0)));
}

TEST(LexerEscape, TurnsABackslashIntoALetter) {
    const std::vector<Token> tokens = tokenize("\\\\");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '\\', at(1, 1, 0)));
}

TEST(LexerEscape, KeepsAnEscapedSpace) {
    const std::vector<Token> tokens = tokenize("\\ ");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, ' ', at(1, 1, 0)));
}

TEST(LexerEscape, LettersAreNotSpecial) {
    // "\n" in the input is a backslash followed by 'n', not a newline.
    const std::vector<Token> tokens = tokenize("\\n");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, 'n', at(1, 1, 0)));
}

TEST(LexerEscape, DanglingBackslashThrows) {
    try {
        tokenize("a\\");
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_EQ(error.position(), at(1, 2, 1));
        EXPECT_STREQ(error.what(), "Dangling backslash");
    }
}

TEST(LexerWhitespace, IsSkippedBetweenTokens) {
    const std::vector<Token> tokens = tokenize(" a\t b ");
    ASSERT_EQ(tokens.size(), 3U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, 'a', at(1, 2, 1)));
    EXPECT_EQ(tokens[1], Token(TokenType::Letter, 'b', at(1, 5, 4)));
    EXPECT_EQ(tokens[2], Token(TokenType::End, '\0', at(1, 7, 6)));
}

TEST(LexerWhitespace, NewlineStartsANewLine) {
    const std::vector<Token> tokens = tokenize("a\nb");
    ASSERT_EQ(tokens.size(), 3U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, 'a', at(1, 1, 0)));
    EXPECT_EQ(tokens[1], Token(TokenType::Letter, 'b', at(2, 1, 2)));
}

TEST(Lexer, KeepsReturningEndPastTheInput) {
    Lexer lexer("a");
    EXPECT_EQ(lexer.next().type(), TokenType::Letter);
    const Token end = lexer.next();
    EXPECT_EQ(end, Token(TokenType::End, '\0', at(1, 2, 1)));
    EXPECT_EQ(lexer.next(), end);
    EXPECT_EQ(lexer.next(), end);
}

} // namespace
} // namespace formal::regex
