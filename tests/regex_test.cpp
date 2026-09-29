#include <sstream>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "regex.hpp"
#include "regex/error.hpp"
#include "regex/position.hpp"
#include "util/alphabet.hpp"

namespace formal {
namespace {

TEST(Regex, PrintsTheCanonicalForm) {
    EXPECT_EQ(Regex("a|bc*").to_string(), "a|bc*");
    EXPECT_EQ(Regex(" (a | b) c ").to_string(), "(a|b)c");
    EXPECT_EQ(Regex("").to_string(), "");
}

TEST(Regex, StreamOutputMatchesToString) {
    const Regex pattern("(a|b)*c");

    std::ostringstream out;
    out << pattern;

    EXPECT_EQ(out.str(), pattern.to_string());
    EXPECT_EQ(out.str(), "(a|b)*c");
}

TEST(Regex, DrawsItselfAsDot) {
    const Regex pattern("ab*");

    EXPECT_EQ(pattern.to_dot(), "digraph regex {\n"
                                "  ordering=out;\n"
                                "  n0 [label=\"Concat\"];\n"
                                "  n1 [label=\"a\"];\n"
                                "  n0 -> n1;\n"
                                "  n2 [label=\"Star\"];\n"
                                "  n3 [label=\"b\"];\n"
                                "  n2 -> n3;\n"
                                "  n0 -> n2;\n"
                                "}\n");
}

TEST(Regex, ConstructionRejectsABadPattern) {
    try {
        const Regex pattern("(a");
        FAIL() << "expected a SyntaxError";
    } catch (const regex::SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Missing ')'");
        EXPECT_EQ(error.position(),
                  (regex::Position{.line = 1, .column = 1, .offset = 0}));
    }
}

TEST(RegexAlphabet, AcceptsAPatternInsideTheAlphabet) {
    const Regex pattern("a|b", Alphabet::from_symbols("ab"));
    EXPECT_EQ(pattern.to_string(), "a|b");
}

TEST(RegexAlphabet, RejectsASymbolOutsideTheAlphabet) {
    try {
        const Regex pattern("abc", Alphabet::from_symbols("ab"));
        FAIL() << "expected a SyntaxError";
    } catch (const regex::SyntaxError& error) {
        EXPECT_STREQ(error.what(), "Symbol 'c' outside the alphabet");
        EXPECT_EQ(error.position(),
                  (regex::Position{.line = 1, .column = 3, .offset = 2}));
    }
}

TEST(RegexAlphabet, LeavesOperatorsAlone) {
    EXPECT_NO_THROW(
        const Regex pattern("(a|b)*", Alphabet::from_symbols("ab")));
}

TEST(RegexAlphabet, WithoutAnAlphabetNothingIsRejected) {
    EXPECT_NO_THROW(const Regex pattern("a|b"));
}

constexpr bool builds_with_an_alphabet() {
    const Regex pattern("a|b", Alphabet::from_symbols("ab"));
    return pattern.to_string() == "a|b";
}

static_assert(builds_with_an_alphabet());

TEST(Regex, SurvivesAMove) {
    Regex original("ab*");
    const Regex moved = std::move(original);

    EXPECT_EQ(moved.to_string(), "ab*");
}

constexpr std::string printed(std::string pattern) {
    return Regex(std::move(pattern)).to_string();
}

static_assert(printed("a|bc*") == "a|bc*");
static_assert(printed("(ab)*") == "(ab)*");

TEST(Regex, ParsesAtRunTimeToo) {
    EXPECT_EQ(printed("a|bc*"), "a|bc*");
}

} // namespace
} // namespace formal
