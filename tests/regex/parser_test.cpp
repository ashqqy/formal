#include <cstddef>
#include <string>

#include <gtest/gtest.h>

#include "regex/error.hpp"
#include "regex/node.hpp"
#include "regex/parser.hpp"
#include "regex/position.hpp"
#include "regex/printer.hpp"
#include "regex/visitor.hpp"

namespace formal::regex {
namespace {

constexpr std::string to_string(const Node& root) {
    Printer printer;
    root.accept(printer);
    return std::move(printer).take();
}


constexpr std::string round_trip(std::string pattern) {
    return to_string(*parse(std::move(pattern)));
}

Position at(std::size_t line, std::size_t column, std::size_t offset) {
    return Position{.line = line, .column = column, .offset = offset};
}

class Counter final : public Visitor {
  public:
    constexpr void visit(const SymbolNode& /*unused*/) override { ++symbols; }
    constexpr void visit(const EpsilonNode& /*unused*/) override { ++epsilons; }

    constexpr void visit(const ConcatNode& node) override {
        ++concats;
        children = node.size();
        visit_children(node);
    }

    constexpr void visit(const UnionNode& node) override {
        ++unions;
        children = node.size();
        visit_children(node);
    }

    constexpr void visit(const StarNode& node) override {
        ++stars;
        node.child().accept(*this);
    }

    int symbols = 0;
    int epsilons = 0;
    int concats = 0;
    int unions = 0;
    int stars = 0;
    std::size_t children = 0;

  private:
    template <class N>
    constexpr void visit_children(const N& node) {
        for (std::size_t i = 0; i < node.size(); ++i) {
            node.child(i).accept(*this);
        }
    }
};

Counter count(const Node& tree) {
    Counter counter;
    tree.accept(counter);
    return counter;
}

struct RoundTripCase {
    const char* input;
    const char* expected;
};

class ParserRoundTrip : public testing::TestWithParam<RoundTripCase> {};

TEST_P(ParserRoundTrip, PrintsBackToTheCanonicalForm) {
    const RoundTripCase round_trip_case = GetParam();
    EXPECT_EQ(round_trip(round_trip_case.input), round_trip_case.expected);
}

INSTANTIATE_TEST_SUITE_P(
    Patterns, ParserRoundTrip,
    testing::Values(
        RoundTripCase{"a", "a"}, RoundTripCase{"abc", "abc"},
        RoundTripCase{"a|b|c", "a|b|c"}, RoundTripCase{"ab|c", "ab|c"},
        RoundTripCase{"(a|b)c", "(a|b)c"}, RoundTripCase{"ab*", "ab*"},
        RoundTripCase{"(ab)*", "(ab)*"}, RoundTripCase{"a**", "a**"},
        RoundTripCase{"(a(b|c))*d", "(a(b|c))*d"}));

TEST(Parser, EmptyPatternIsEpsilon) {
    EXPECT_EQ(round_trip(""), "");
    EXPECT_EQ(count(*parse("")).epsilons, 1);
}

TEST(Parser, EmptyGroupIsEpsilon) {
    EXPECT_EQ(round_trip("()"), "");
    EXPECT_EQ(count(*parse("()")).epsilons, 1);
}

TEST(Parser, EmptyBranchIsEpsilon) {
    EXPECT_EQ(round_trip("(a|)"), "a|");

    const Counter counter = count(*parse("(a|)"));
    EXPECT_EQ(counter.unions, 1);
    EXPECT_EQ(counter.symbols, 1);
    EXPECT_EQ(counter.epsilons, 1);
}

TEST(Parser, GroupsDoNotCreateNodes) {
    EXPECT_EQ(round_trip("(((a)))"), "a");
    EXPECT_EQ(count(*parse("(((a)))")).symbols, 1);
}

TEST(Parser, BuildsTheSameTreeAsTheFactories) {
    EXPECT_EQ(to_string(*parse("ab")),
              to_string(*make_concat(make_symbol('a'), make_symbol('b'))));
    EXPECT_EQ(to_string(*parse("a|b")),
              to_string(*make_union(make_symbol('a'), make_symbol('b'))));
    EXPECT_EQ(to_string(*parse("a*")), to_string(*make_star(make_symbol('a'))));
}

TEST(Parser, ConcatenationIsFlat) {
    const Counter counter = count(*parse("abc"));
    EXPECT_EQ(counter.concats, 1);
    EXPECT_EQ(counter.children, 3U);
    EXPECT_EQ(counter.symbols, 3);
}

TEST(Parser, AlternationIsFlat) {
    const Counter counter = count(*parse("a|b|c"));
    EXPECT_EQ(counter.unions, 1);
    EXPECT_EQ(counter.children, 3U);
    EXPECT_EQ(counter.symbols, 3);
}

TEST(Parser, StarBindsTighterThanConcatenation) {
    const Counter counter = count(*parse("ab*"));
    EXPECT_EQ(counter.concats, 1);
    EXPECT_EQ(counter.stars, 1);
    EXPECT_EQ(counter.children, 2U);
}

TEST(ParserLexerSeam, KeepsEscapedOperatorsAsSymbols) {
    EXPECT_EQ(round_trip("\\|"), "\\|");
    EXPECT_EQ(count(*parse("\\|")).unions, 0);
    EXPECT_EQ(count(*parse("\\*")).stars, 0);
}

TEST(ParserLexerSeam, IgnoresWhitespace) {
    EXPECT_EQ(round_trip(" a | b "), "a|b");
    EXPECT_EQ(round_trip("( a b )*"), "(ab)*");
}

std::string error_message(std::string pattern) {
    try {
        (void)parse(std::move(pattern));
    } catch (const SyntaxError& error) { return error.what(); }
    return {};
}

TEST(ParserError, ReportsAnUnclosedGroupAtItsOpeningParen) {
    try {
        (void)parse("(ab");
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Missing ')'");
        EXPECT_EQ(error.position(), at(1, 1, 0));
    }
}

TEST(ParserError, ReportsATrailingToken) {
    try {
        (void)parse("a)");
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Unmatched ')'");
        EXPECT_EQ(error.position(), at(1, 2, 1));
    }
}

TEST(ParserError, ReportsAStarWithoutAnExpression) {
    try {
        (void)parse("*a");
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Nothing to repeat before '*'");
        EXPECT_EQ(error.position(), at(1, 1, 0));
    }
}

TEST(ParserError, LetsALexerErrorThrough) {
    try {
        (void)parse("a\\");
        FAIL() << "expected a SyntaxError";
    } catch (const SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Nothing to escape after '\\'");
        EXPECT_EQ(error.position(), at(1, 2, 1));
    }
}

TEST(ParserError, TellsUnexpectedTokensApart) {
    EXPECT_NE(error_message("a)"), error_message("*a"));
}

static_assert(round_trip("a|bc*") == "a|bc*");
static_assert(round_trip("(a|b)*c") == "(a|b)*c");
static_assert(round_trip("(a|)") == "a|");

TEST(Parser, ParsesAtRunTimeToo) {
    EXPECT_EQ(round_trip("a|bc*"), "a|bc*");
}

} // namespace
} // namespace formal::regex
