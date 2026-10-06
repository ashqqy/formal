#include <cstddef>
#include <string>

#include <gtest/gtest.h>

#include "automata/dot.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/parser.hpp"
#include "regex/thompson.hpp"
#include "util/symbol.hpp"

namespace formal::regex {
namespace {

constexpr automata::Nfa nfa_of(std::string pattern) {
    return to_nfa(*parse(std::move(pattern)));
}

constexpr std::size_t states_of(std::string pattern) {
    return nfa_of(std::move(pattern)).size();
}

static_assert(states_of("a") == 2);
static_assert(states_of("()") == 2);
static_assert(states_of("ab") == 4);
static_assert(states_of("a|b") == 6);
static_assert(states_of("a*") == 4);

TEST(Thompson, BuildsTwoStatesForASymbol) {
    const automata::Nfa nfa = nfa_of("a");

    ASSERT_EQ(nfa.size(), 2U);
    ASSERT_EQ(nfa.starts().size(), 1U);
    const automata::StateId start = nfa.starts()[0];

    ASSERT_EQ(nfa.transitions(start).size(), 1U);
    EXPECT_EQ(nfa.transitions(start)[0].symbol, to_symbol('a'));
    EXPECT_TRUE(nfa.is_accepting(nfa.transitions(start)[0].target));
    EXPECT_FALSE(nfa.is_accepting(start));
}

TEST(Thompson, BuildsTwoStatesForEpsilon) {
    const automata::Nfa nfa = nfa_of("()");

    ASSERT_EQ(nfa.size(), 2U);
    const automata::StateId start = nfa.starts()[0];
    EXPECT_TRUE(nfa.transitions(start).empty());
    ASSERT_EQ(nfa.epsilons(start).size(), 1U);
    EXPECT_TRUE(nfa.is_accepting(nfa.epsilons(start)[0]));
}

TEST(Thompson, AddsNoStatesForConcatenation) {
    EXPECT_EQ(states_of("ab"), 4U);
    EXPECT_EQ(states_of("abc"), 6U);
    EXPECT_EQ(states_of("abcd"), 8U);
}

TEST(Thompson, AddsOnePairPerUnionWhateverItsArity) {
    EXPECT_EQ(states_of("a|b"), 6U);
    EXPECT_EQ(states_of("a|b|c"), 8U);
    EXPECT_EQ(states_of("a|b|c|d"), 10U);
}

TEST(Thompson, AddsOnePairPerStar) {
    EXPECT_EQ(states_of("a*"), 4U);
    EXPECT_EQ(states_of("a**"), 6U);
}

TEST(Thompson, GivesAStarFourEpsilonEdges) {
    const automata::Nfa nfa = nfa_of("a*");
    const automata::StateId start = nfa.starts()[0];

    ASSERT_EQ(nfa.size(), 4U);
    EXPECT_EQ(nfa.epsilons(start).size(), 2U);

    std::size_t total = 0;
    for (automata::StateId id = 0; id < nfa.size(); ++id) {
        total += nfa.epsilons(id).size();
    }
    EXPECT_EQ(total, 4U);
}

TEST(Thompson, TerminatesOnNestedStars) {
    EXPECT_EQ(states_of("(a*)*"), 6U);
    EXPECT_EQ(states_of("((a*)*)*"), 8U);
}

TEST(Thompson, MarksExactlyOneStartAndOneAcceptingState) {
    const automata::Nfa nfa = nfa_of("a|bc*");

    EXPECT_EQ(nfa.starts().size(), 1U);

    std::size_t accepting = 0;
    for (automata::StateId id = 0; id < nfa.size(); ++id) {
        accepting += nfa.is_accepting(id) ? 1U : 0U;
    }
    EXPECT_EQ(accepting, 1U);
}

TEST(Thompson, KeepsTheAlphabetOfTheSymbols) {
    EXPECT_EQ(nfa_of("a|bc*").alphabet(), Alphabet::from_symbols("abc"));
    EXPECT_EQ(nfa_of("()").alphabet(), Alphabet{});
}

TEST(Thompson, BuildsTheWholeAutomatonOfAPattern) {
    EXPECT_EQ(to_dot(nfa_of("a|bc*")),
              "digraph nfa {\n"
              "  rankdir=LR;\n"
              "  start0 [shape=point, width=0];\n"
              "  start0 -> n0;\n"
              "  n0 [label=\"0\"];\n"
              "  n1 [label=\"1\", shape=doublecircle];\n"
              "  n2 [label=\"2\"];\n"
              "  n3 [label=\"3\"];\n"
              "  n4 [label=\"4\"];\n"
              "  n5 [label=\"5\"];\n"
              "  n6 [label=\"6\"];\n"
              "  n7 [label=\"7\"];\n"
              "  n8 [label=\"8\"];\n"
              "  n9 [label=\"9\"];\n"
              "  n0 -> n2 [label=\"eps\"];\n"
              "  n0 -> n4 [label=\"eps\"];\n"
              "  n2 -> n3 [label=\"a\"];\n"
              "  n3 -> n1 [label=\"eps\"];\n"
              "  n4 -> n5 [label=\"b\"];\n"
              "  n5 -> n6 [label=\"eps\"];\n"
              "  n6 -> n7 [label=\"eps\"];\n"
              "  n6 -> n8 [label=\"eps\"];\n"
              "  n7 -> n1 [label=\"eps\"];\n"
              "  n8 -> n9 [label=\"c\"];\n"
              "  n9 -> n7 [label=\"eps\"];\n"
              "  n9 -> n8 [label=\"eps\"];\n"
              "}\n");
}

} // namespace
} // namespace formal::regex
