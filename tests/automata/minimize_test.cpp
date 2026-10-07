#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "automata/complete.hpp"
#include "automata/determinize.hpp"
#include "automata/dfa.hpp"
#include "automata/matches.hpp"
#include "automata/minimize.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/parser.hpp"
#include "regex/thompson.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr Dfa minimal(const char* pattern) {
    return minimize(determinize(regex::to_nfa(*regex::parse(pattern))));
}

constexpr std::size_t states(const char* pattern) {
    return minimal(pattern).size();
}

static_assert(states("a*") == 1);
static_assert(states("(a|b)*a") == 2);
static_assert(states("a") == 3);
static_assert(is_complete(minimal("ab")));
static_assert(minimal("a*") == minimal("a**"));
static_assert(matches(minimal("a|bc*"), "bcc"));
static_assert(!matches(minimal("a|bc*"), "cc"));

struct CountCase {
    const char* pattern;
    std::size_t states;
};

class MinimizeCount : public testing::TestWithParam<CountCase> {};

TEST_P(MinimizeCount, ReachesTheKnownMinimum) {
    const CountCase count_case = GetParam();
    EXPECT_EQ(states(count_case.pattern), count_case.states)
        << "pattern " << count_case.pattern;
}

INSTANTIATE_TEST_SUITE_P(
    Patterns, MinimizeCount,
    testing::Values(
        CountCase{"a*", 1}, CountCase{"(a*)*", 1}, CountCase{"a**", 1},
        CountCase{"(a|b)*", 1}, CountCase{"()", 1}, CountCase{"()*", 1},
        CountCase{"(a|b)*a", 2}, CountCase{"a", 3}, CountCase{"a|b", 3},
        CountCase{"a|a", 3}, CountCase{"(a|)", 3}, CountCase{"a|b|c", 3},
        CountCase{"(a|b)*c", 3}, CountCase{"(ab)*", 3}, CountCase{"ab", 4},
        CountCase{"a|bc*", 4}, CountCase{"abc", 5}));

TEST(Minimize, MergesEquivalentStates) {
    const Dfa dfa = determinize(regex::to_nfa(*regex::parse("a|bc*")));

    EXPECT_EQ(complete(dfa).size(), 5U);
    EXPECT_EQ(minimize(dfa).size(), 4U);
}

TEST(Minimize, ReturnsACompleteAutomaton) {
    for (const char* pattern : {"a", "ab", "a|bc*", "(ab)*", "abc"}) {
        EXPECT_TRUE(is_complete(minimal(pattern))) << "pattern " << pattern;
    }
}

TEST(Minimize, IsIdempotent) {
    for (const char* pattern :
         {"a", "a*", "ab", "abc", "a|bc*", "(a|b)*c", "(ab)*", "a|b|c"}) {
        const Dfa once = minimal(pattern);
        EXPECT_EQ(minimize(once), once) << "pattern " << pattern;
    }
}

TEST(Minimize, SetsTheStart) {
    const Dfa dfa = minimal("a|bc*");
    EXPECT_NE(dfa.start(), kNoState);
    EXPECT_LT(dfa.start(), dfa.size());
}

TEST(Minimize, CarriesTheAlphabetOver) {
    const Nfa nfa = regex::to_nfa(*regex::parse("a|bc*"));
    EXPECT_EQ(minimize(determinize(nfa)).alphabet(), nfa.alphabet());
}

TEST(Minimize, GivesACanonicalForm) {
    EXPECT_EQ(minimal("a*"), minimal("a**"));
    EXPECT_EQ(minimal("a*"), minimal("(a*)*"));
    EXPECT_EQ(minimal("a|b"), minimal("b|a"));
    EXPECT_EQ(minimal("a|a"), minimal("a"));
    EXPECT_EQ(minimal("(a|b)*"), minimal("(b|a)*"));
    EXPECT_EQ(minimal("(a|b)*a"), minimal("(b|a)*a"));
}

TEST(Minimize, TellsDifferentLanguagesApart) {
    EXPECT_NE(minimal("a*"), minimal("a"));
    EXPECT_NE(minimal("ab"), minimal("ba"));
    EXPECT_NE(minimal("a|b"), minimal("ab"));
}

TEST(Minimize, OnlyCompletesAnAutomatonWithoutAStart) {
    Dfa dfa{Alphabet::from_symbols("a")};
    const StateId state = dfa.add_state();
    dfa.set_accepting(state);

    EXPECT_EQ(dfa.start(), kNoState);
    EXPECT_EQ(minimize(dfa), complete(dfa));
    EXPECT_EQ(minimize(dfa).start(), kNoState);
}

TEST(Minimize, CollapsesAnAutomatonThatAcceptsEverything) {
    const Dfa dfa = minimal("(a|b)*");

    EXPECT_EQ(dfa.size(), 1U);
    EXPECT_TRUE(dfa.is_accepting(dfa.start()));
    for (const Symbol symbol : dfa.alphabet()) {
        EXPECT_EQ(dfa.transition(dfa.start(), symbol), dfa.start());
    }
}

TEST(Minimize, KeepsTheLanguage) {
    constexpr std::string_view inputs[] = {
        "",    "a",    "b",   "c",   "aa",  "ab", "ba", "bc",    "cc",
        "bcc", "abab", "abc", "aaa", "bca", "cb", "z",  "bcccc", "aab"};

    for (const char* pattern :
         {"a", "a*", "(a*)*", "a**", "ab", "abc", "(ab)*", "a|b", "a|b|c",
          "(a|b)*a", "a|bc*", "(a|b)*c", "()", "()*", "(a|)", "(a|b)*"}) {
        const Nfa nfa = regex::to_nfa(*regex::parse(pattern));
        const Dfa dfa = determinize(nfa);
        const Dfa small = minimize(dfa);

        for (const std::string_view input : inputs) {
            EXPECT_EQ(matches(small, input), matches(nfa, input))
                << "pattern " << pattern << ", input \"" << input << '"';
            EXPECT_EQ(matches(small, input), matches(dfa, input))
                << "pattern " << pattern << ", input \"" << input << '"';
        }
    }
}

} // namespace
} // namespace formal::automata
