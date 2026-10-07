#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "automata/determinize.hpp"
#include "automata/dfa.hpp"
#include "automata/matches.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/parser.hpp"
#include "regex/thompson.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr Dfa determinized(const char* pattern) {
    return determinize(regex::to_nfa(*regex::parse(pattern)));
}

constexpr std::size_t states(const char* pattern) {
    return determinized(pattern).size();
}

static_assert(states("a") == 2);
static_assert(states("a*") == 2);
static_assert(states("ab") == 3);
static_assert(states("()") == 1);
static_assert(matches(determinized("a|bc*"), "bcc"));
static_assert(!matches(determinized("a|bc*"), "cc"));

struct CountCase {
    const char* pattern;
    std::size_t states;
};

class DeterminizeCount : public testing::TestWithParam<CountCase> {};

TEST_P(DeterminizeCount, ProducesOnlyReachableSubsets) {
    const CountCase count_case = GetParam();
    EXPECT_EQ(states(count_case.pattern), count_case.states)
        << "pattern " << count_case.pattern;
}

INSTANTIATE_TEST_SUITE_P(
    Patterns, DeterminizeCount,
    testing::Values(
        CountCase{"a", 2}, CountCase{"a*", 2}, CountCase{"(a*)*", 2},
        CountCase{"a**", 2}, CountCase{"ab", 3}, CountCase{"abc", 4},
        CountCase{"(ab)*", 3}, CountCase{"a|b", 3}, CountCase{"a|b|c", 4},
        CountCase{"(a|b)*a", 3}, CountCase{"a|bc*", 4}, CountCase{"(a|b)*c", 4},
        CountCase{"()", 1}, CountCase{"()*", 1}, CountCase{"(a|)", 2}));

TEST(Determinize, SetsTheStart) {
    const Dfa dfa = determinized("a|bc*");
    EXPECT_NE(dfa.start(), kNoState);
    EXPECT_LT(dfa.start(), dfa.size());
}

TEST(Determinize, CarriesTheAlphabetOver) {
    const Nfa nfa = regex::to_nfa(*regex::parse("a|bc*"));
    EXPECT_EQ(determinize(nfa).alphabet(), nfa.alphabet());
}

TEST(Determinize, KeepsTheWholeDeclaredAlphabet) {
    Nfa nfa;
    const StateId state = nfa.add_state();
    nfa.add_start(state);
    nfa.widen_alphabet(Alphabet::from_symbols("xyz"));

    EXPECT_EQ(determinize(nfa).alphabet(), Alphabet::from_symbols("xyz"));
}

TEST(Determinize, LeavesHolesInsteadOfASink) {
    const Dfa dfa = determinized("ab");

    std::size_t holes = 0;
    for (StateId id = 0; id < dfa.size(); ++id) {
        for (const Symbol symbol : dfa.alphabet()) {
            if (dfa.transition(id, symbol) == kNoState) { ++holes; }
        }
    }
    EXPECT_GT(holes, 0U);
}

TEST(Determinize, MarksAcceptingSubsets) {
    const Dfa dfa = determinized("a");

    EXPECT_FALSE(dfa.is_accepting(dfa.start()));
    EXPECT_TRUE(dfa.is_accepting(dfa.transition(dfa.start(), to_symbol('a'))));
}

TEST(Determinize, AcceptsTheEmptyStringThroughTheStart) {
    const Dfa dfa = determinized("a*");
    EXPECT_TRUE(dfa.is_accepting(dfa.start()));
}

TEST(Determinize, ReusesAStateForARepeatedSubset) {
    EXPECT_EQ(states("a|a"), states("a"));
    EXPECT_EQ(states("(a|a)*"), states("a*"));
}

TEST(Determinize, IsReproducible) {
    const Nfa nfa = regex::to_nfa(*regex::parse("(a|b)*c"));
    EXPECT_EQ(determinize(nfa), determinize(nfa));
}

TEST(Determinize, OfAnEmptyAutomatonHasOneState) {
    const Dfa dfa = determinize(Nfa{});

    EXPECT_EQ(dfa.size(), 1U);
    EXPECT_EQ(dfa.start(), 0U);
    EXPECT_FALSE(dfa.is_accepting(0));
    EXPECT_FALSE(matches(dfa, ""));
}

TEST(Determinize, RemovesEveryEpsilon) {
    const Dfa dfa = determinized("(a*)*");
    EXPECT_EQ(dfa.size(), 2U);
    EXPECT_TRUE(matches(dfa, ""));
    EXPECT_TRUE(matches(dfa, "aaa"));
}

TEST(Determinize, KeepsTheLanguage) {
    constexpr std::string_view inputs[] = {
        "",   "a",   "b",   "c",    "aa",  "ab",  "ba", "bc",
        "cc", "bcc", "abc", "abab", "aaa", "bca", "cb", "z"};

    for (const char* pattern :
         {"a", "a*", "(a*)*", "a**", "ab", "abc", "(ab)*", "a|b", "a|b|c",
          "(a|b)*a", "a|bc*", "(a|b)*c", "()", "()*", "(a|)", "(a|b)*"}) {
        const Nfa nfa = regex::to_nfa(*regex::parse(pattern));
        const Dfa dfa = determinize(nfa);

        for (const std::string_view input : inputs) {
            EXPECT_EQ(matches(dfa, input), matches(nfa, input))
                << "pattern " << pattern << ", input \"" << input << '"';
        }
    }
}

} // namespace
} // namespace formal::automata
