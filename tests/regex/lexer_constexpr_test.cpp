#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "regex/error.hpp"
#include "regex/lexer.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"

namespace formal::regex {
namespace {

// The same helpers as in lexer_test.cpp, but usable inside a constant
// expression: everything they touch is destroyed before the expression ends.
constexpr std::vector<Token> tokenize(std::string input) {
    Lexer lexer(std::move(input));
    std::vector<Token> tokens;
    for (;;) {
        Token token = lexer.next();
        tokens.push_back(token);
        if (token.type() == TokenType::End) { return tokens; }
    }
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

static_assert(token_count("a*(b+1)") == 8);
static_assert(type_at("a*(b+1)", 0) == TokenType::Letter);
static_assert(type_at("a*(b+1)", 1) == TokenType::Star);
static_assert(type_at("a*(b+1)", 2) == TokenType::Lparen);
static_assert(type_at("a*(b+1)", 3) == TokenType::Letter);
static_assert(type_at("a*(b+1)", 4) == TokenType::Plus);
static_assert(type_at("a*(b+1)", 5) == TokenType::Epsilon);
static_assert(type_at("a*(b+1)", 6) == TokenType::Rparen);
static_assert(type_at("a*(b+1)", 7) == TokenType::End);

static_assert(token_at("a*(b+1)", 3) ==
              Token(TokenType::Letter, 'b',
                    Position{.line = 1, .column = 4, .offset = 3}));

static_assert(token_at("\\*", 0) ==
              Token(TokenType::Letter, '*',
                    Position{.line = 1, .column = 1, .offset = 0}));
static_assert(token_at(" a\nb", 1) ==
              Token(TokenType::Letter, 'b',
                    Position{.line = 2, .column = 1, .offset = 3}));

// A SyntaxError thrown and caught inside a constant expression (C++26).
static_assert(position_of_dangling_backslash("a\\") ==
              Position{.line = 1, .column = 2, .offset = 1});

// The same checks at run time, so that they also show up in coverage.
TEST(LexerConstexpr, MatchesRuntimeLexing) {
    EXPECT_EQ(token_count("a*(b+1)"), 8U);
    EXPECT_EQ(type_at("a*(b+1)", 1), TokenType::Star);
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
