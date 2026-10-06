#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "regex/node.hpp"
#include "regex/printer.hpp"

namespace formal::regex {
namespace {


TEST(Printer, PrintsASymbol) {
    EXPECT_EQ(to_string(*make_symbol('a')), "a");
}

TEST(Printer, PrintsEpsilonAsNothing) {
    EXPECT_EQ(to_string(*make_epsilon()), "");
}

TEST(Printer, StarBindsTighterThanConcat) {
    const NodePtr tree =
        make_concat(make_symbol('a'), make_star(make_symbol('b')));
    EXPECT_EQ(to_string(*tree), "ab*");
}

TEST(Printer, ConcatUnderStarNeedsParens) {
    const NodePtr tree =
        make_star(make_concat(make_symbol('a'), make_symbol('b')));
    EXPECT_EQ(to_string(*tree), "(ab)*");
}

TEST(Printer, ConcatBindsTighterThanUnion) {
    const NodePtr tree = make_union(
        make_symbol('a'), make_concat(make_symbol('b'), make_symbol('c')));
    EXPECT_EQ(to_string(*tree), "a|bc");
}

TEST(Printer, UnionUnderConcatNeedsParens) {
    const NodePtr tree = make_concat(
        make_union(make_symbol('a'), make_symbol('b')), make_symbol('c'));
    EXPECT_EQ(to_string(*tree), "(a|b)c");
}

TEST(Printer, StarUnderStarNeedsNoParens) {
    EXPECT_EQ(to_string(*make_star(make_star(make_symbol('a')))), "a**");
}

TEST(Printer, EpsilonUnderStarBecomesAnEmptyGroup) {
    EXPECT_EQ(to_string(*make_star(make_epsilon())), "()*");
}

TEST(Printer, EpsilonInAUnionBranchStaysEmpty) {
    const NodePtr tree = make_union(make_symbol('a'), make_epsilon());
    EXPECT_EQ(to_string(*tree), "a|");
}

TEST(Printer, EscapesOperators) {
    EXPECT_EQ(to_string(*make_symbol('|')), "\\|");
    EXPECT_EQ(to_string(*make_symbol('*')), "\\*");
    EXPECT_EQ(to_string(*make_symbol('(')), "\\(");
    EXPECT_EQ(to_string(*make_symbol(')')), "\\)");
    EXPECT_EQ(to_string(*make_symbol('\\')), "\\\\");
    EXPECT_EQ(to_string(*make_symbol(' ')), "\\ ");
}

TEST(Printer, SeparatesEveryUnionBranch) {
    const NodePtr tree = make_union(make_symbol('a'), make_symbol('b'),
                                    make_symbol('c'), make_symbol('d'));
    EXPECT_EQ(to_string(*tree), "a|b|c|d");
}

TEST(Printer, ConcatenatesEveryFactor) {
    const NodePtr tree = make_concat(make_symbol('a'), make_symbol('b'),
                                     make_symbol('c'), make_symbol('d'));
    EXPECT_EQ(to_string(*tree), "abcd");
}

TEST(Printer, ParenthesisesAFlatUnionUnderStar) {
    const NodePtr tree = make_star(
        make_union(make_symbol('a'), make_symbol('b'), make_symbol('c')));
    EXPECT_EQ(to_string(*tree), "(a|b|c)*");
}

TEST(Printer, MixesFlatUnionAndConcat) {
    const NodePtr tree = make_union(
        make_concat(make_symbol('a'), make_symbol('b')), make_symbol('c'),
        make_concat(make_symbol('d'), make_star(make_symbol('e'))));
    EXPECT_EQ(to_string(*tree), "ab|c|de*");
}

TEST(Printer, KeepsParensOfNestedGroups) {
    const NodePtr tree = make_star(make_union(
        make_concat(make_symbol('a'), make_symbol('b')), make_symbol('c')));
    EXPECT_EQ(to_string(*tree), "(ab|c)*");
}


constexpr bool prints_at_compile_time() {
    return to_string(*make_star(
               make_concat(make_symbol('a'), make_symbol('b')))) == "(ab)*" &&
           to_string(*make_union(make_symbol('a'), make_epsilon())) == "a|";
}

static_assert(prints_at_compile_time());

TEST(Printer, PrintsAtRunTimeToo) {
    EXPECT_TRUE(prints_at_compile_time());
}

} // namespace
} // namespace formal::regex
