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
// Usable in a constant expression: everything it touches dies with it.
constexpr std::vector<Token> tokenize(std::string input) {
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

TEST(Lexer, NonOperatorsAreLetters) {
    const std::vector<Token> tokens = tokenize("01.+?");
    ASSERT_EQ(tokens.size(), 6U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '0', at(1, 1, 0)));
    EXPECT_EQ(tokens[1], Token(TokenType::Letter, '1', at(1, 2, 1)));
    EXPECT_EQ(tokens[2], Token(TokenType::Letter, '.', at(1, 3, 2)));
    EXPECT_EQ(tokens[3], Token(TokenType::Letter, '+', at(1, 4, 3)));
    EXPECT_EQ(tokens[4], Token(TokenType::Letter, '?', at(1, 5, 4)));
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
    EXPECT_EQ(tokens[0], Token(operator_case.type,
                               to_symbol(operator_case.input), at(1, 1, 0)));
}

INSTANTIATE_TEST_SUITE_P(
    Operators, LexerOperator,
    testing::Values(OperatorCase{'|', TokenType::Pipe},
                    OperatorCase{'*', TokenType::Star},
                    OperatorCase{'(', TokenType::Lparen},
                    OperatorCase{')', TokenType::Rparen}),
    [](const testing::TestParamInfo<OperatorCase>& test_case) {
        return std::string(enum_name(test_case.param.type));
    });

TEST(LexerEscape, TurnsAnOperatorIntoALetter) {
    const std::vector<Token> tokens = tokenize("\\*");
    ASSERT_EQ(tokens.size(), 2U);
    // The position points at the backslash, not at the escaped symbol.
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '*', at(1, 1, 0)));
}

TEST(LexerEscape, TurnsAPipeIntoALetter) {
    const std::vector<Token> tokens = tokenize("\\|");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], Token(TokenType::Letter, '|', at(1, 1, 0)));
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

TEST(LexerPeek, DoesNotConsume) {
    Lexer lexer("ab");

    const Token first = lexer.peek();
    EXPECT_EQ(lexer.peek(), first);
    EXPECT_EQ(lexer.next(), first);
    EXPECT_EQ(lexer.peek(), Token(TokenType::Letter, 'b', at(1, 2, 1)));
}

TEST(LexerPeek, SkipsWhitespaceLikeNext) {
    Lexer lexer(" \t a");
    EXPECT_EQ(lexer.peek(), Token(TokenType::Letter, 'a', at(1, 4, 3)));
    EXPECT_EQ(lexer.next(), lexer.peek());
}

TEST(LexerPeek, KeepsReturningEndPastTheInput) {
    Lexer lexer("");
    const Token end = Token(TokenType::End, '\0', at(1, 1, 0));
    EXPECT_EQ(lexer.peek(), end);
    EXPECT_EQ(lexer.next(), end);
    EXPECT_EQ(lexer.peek(), end);
}

TEST(LexerPeek, ReportsADanglingBackslash) {
    Lexer lexer("a\\");
    EXPECT_EQ(lexer.next().type(), TokenType::Letter);
    try {
        (void)lexer.peek();
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_EQ(error.position(), at(1, 2, 1));
    }
}

TEST(Lexer, KeepsReturningEndPastTheInput) {
    Lexer lexer("a");
    EXPECT_EQ(lexer.next().type(), TokenType::Letter);
    const Token end = lexer.next();
    EXPECT_EQ(end, Token(TokenType::End, '\0', at(1, 2, 1)));
    EXPECT_EQ(lexer.next(), end);
    EXPECT_EQ(lexer.next(), end);
}

constexpr std::size_t token_count(std::string input) {
    return tokenize(std::move(input)).size();
}

constexpr TokenType type_at(std::string input, std::size_t index) {
    return tokenize(std::move(input))[index].type();
}

constexpr Token token_at(std::string input, std::size_t index) {
    return tokenize(std::move(input))[index];
}

constexpr Position position_of_dangling_backslash(std::string input) {
    try {
        tokenize(std::move(input));
    } catch (const SyntaxError& error) { return error.position(); }
    return Position{.line = 0, .column = 0, .offset = 0};
}

static_assert(token_count("a*(b|c)") == 8);
static_assert(type_at("a*(b|c)", 0) == TokenType::Letter);
static_assert(type_at("a*(b|c)", 1) == TokenType::Star);
static_assert(type_at("a*(b|c)", 2) == TokenType::Lparen);
static_assert(type_at("a*(b|c)", 3) == TokenType::Letter);
static_assert(type_at("a*(b|c)", 4) == TokenType::Pipe);
static_assert(type_at("a*(b|c)", 5) == TokenType::Letter);
static_assert(type_at("a*(b|c)", 6) == TokenType::Rparen);
static_assert(type_at("a*(b|c)", 7) == TokenType::End);

static_assert(token_at("a*(b|c)", 3) ==
              Token(TokenType::Letter, 'b',
                    Position{.line = 1, .column = 4, .offset = 3}));

static_assert(token_at("\\*", 0) ==
              Token(TokenType::Letter, '*',
                    Position{.line = 1, .column = 1, .offset = 0}));
static_assert(token_at(" a\nb", 1) ==
              Token(TokenType::Letter, 'b',
                    Position{.line = 2, .column = 1, .offset = 3}));

constexpr bool peek_agrees_with_next(std::string input) {
    Lexer lexer(std::move(input));
    const Token peeked = lexer.peek();
    return peeked == lexer.peek() && peeked == lexer.next();
}

static_assert(peek_agrees_with_next("a|b"));
static_assert(peek_agrees_with_next(""));

// A SyntaxError thrown and caught inside a constant expression (C++26).
static_assert(position_of_dangling_backslash("a\\") ==
              Position{.line = 1, .column = 2, .offset = 1});

// The same checks at run time, so that they also show up in coverage.
TEST(LexerConstexpr, MatchesRuntimeLexing) {
    EXPECT_EQ(token_count("a*(b|c)"), 8U);
    EXPECT_EQ(type_at("a*(b|c)", 1), TokenType::Star);
    EXPECT_EQ(token_at("\\*", 0),
              Token(TokenType::Letter, '*',
                    Position{.line = 1, .column = 1, .offset = 0}));
}

TEST(LexerConstexpr, ReportsTheSameErrorPosition) {
    EXPECT_EQ(position_of_dangling_backslash("a\\"),
              (Position{.line = 1, .column = 2, .offset = 1}));
}

} // namespace
} // namespace formal::regex
