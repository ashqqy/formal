#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "automata/matches.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/parser.hpp"
#include "regex/thompson.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr bool accepts(const char* pattern, std::string_view input) {
    return matches(regex::to_nfa(*regex::parse(pattern)), input);
}

static_assert(accepts("a*", ""));
static_assert(accepts("a*", "aaa"));
static_assert(accepts("(a*)*", "aaa"));
static_assert(accepts("a|bc*", "bcc"));
static_assert(!accepts("ab", "abc"));

struct MatchCase {
    const char* pattern;
    const char* input;
    bool expected;
};

class Matches : public testing::TestWithParam<MatchCase> {};

TEST_P(Matches, AgreesWithTheLanguage) {
    const MatchCase match_case = GetParam();
    EXPECT_EQ(accepts(match_case.pattern, match_case.input),
              match_case.expected)
        << "pattern " << match_case.pattern << ", input " << match_case.input;
}

INSTANTIATE_TEST_SUITE_P(
    Star, Matches,
    testing::Values(MatchCase{"a*", "", true}, MatchCase{"a*", "a", true},
                    MatchCase{"a*", "aaa", true}, MatchCase{"a*", "b", false},
                    MatchCase{"a*", "ab", false}, MatchCase{"(ab)*", "", true},
                    MatchCase{"(ab)*", "abab", true},
                    MatchCase{"(ab)*", "aba", false}));

INSTANTIATE_TEST_SUITE_P(EpsilonCycles, Matches,
                         testing::Values(MatchCase{"()*", "", true},
                                         MatchCase{"()*", "a", false},
                                         MatchCase{"(a*)*", "", true},
                                         MatchCase{"(a*)*", "aaa", true},
                                         MatchCase{"(a*)*", "b", false},
                                         MatchCase{"a**", "aa", true},
                                         MatchCase{"(a|)", "", true},
                                         MatchCase{"(a|)", "a", true}));

INSTANTIATE_TEST_SUITE_P(Empty, Matches,
                         testing::Values(MatchCase{"", "", true},
                                         MatchCase{"", "a", false},
                                         MatchCase{"()", "", true},
                                         MatchCase{"a", "", false}));

INSTANTIATE_TEST_SUITE_P(Union, Matches,
                         testing::Values(MatchCase{"a|b|c", "b", true},
                                         MatchCase{"a|b|c", "d", false},
                                         MatchCase{"a|bc*", "a", true},
                                         MatchCase{"a|bc*", "b", true},
                                         MatchCase{"a|bc*", "bcc", true},
                                         MatchCase{"a|bc*", "ac", false},
                                         MatchCase{"(a|b)*c", "abbac", true},
                                         MatchCase{"(a|b)*c", "abba", false}));

INSTANTIATE_TEST_SUITE_P(WholeInputOnly, Matches,
                         testing::Values(MatchCase{"ab", "ab", true},
                                         MatchCase{"ab", "a", false},
                                         MatchCase{"ab", "abc", false},
                                         MatchCase{"ab", "zab", false},
                                         MatchCase{"abc", "abc", true}));

INSTANTIATE_TEST_SUITE_P(Escapes, Matches,
                         testing::Values(MatchCase{"\\*", "*", true},
                                         MatchCase{"\\*", "", false},
                                         MatchCase{"a\\|b", "a|b", true}));

TEST(MatchesNfa, AnAutomatonWithoutStatesAcceptsNothing) {
    EXPECT_FALSE(matches(Nfa{}, ""));
    EXPECT_FALSE(matches(Nfa{}, "a"));
}

TEST(MatchesNfa, AStartThatIsAcceptingTakesTheEmptyString) {
    Nfa nfa;
    const StateId start = nfa.add_state();
    nfa.add_start(start);

    EXPECT_FALSE(matches(nfa, ""));
    nfa.set_accepting(start);
    EXPECT_TRUE(matches(nfa, ""));
}

TEST(MatchesNfa, WithoutAStartAcceptsNothing) {
    Nfa nfa;
    const StateId state = nfa.add_state();
    nfa.set_accepting(state);

    EXPECT_FALSE(matches(nfa, ""));
}

TEST(MatchesNfa, FollowsEveryBranch) {
    Nfa nfa;
    const StateId start = nfa.add_state();
    const StateId left = nfa.add_state();
    const StateId right = nfa.add_state();
    nfa.add_start(start);
    nfa.add_transition(start, to_symbol('a'), left);
    nfa.add_transition(start, to_symbol('a'), right);
    nfa.set_accepting(right);

    EXPECT_TRUE(matches(nfa, "a"));
}

TEST(MatchesNfa, SeveralStartsAreAllTried) {
    Nfa nfa;
    const StateId first = nfa.add_state();
    const StateId second = nfa.add_state();
    const StateId accept = nfa.add_state();
    nfa.add_start(first);
    nfa.add_start(second);
    nfa.add_transition(second, to_symbol('a'), accept);
    nfa.set_accepting(accept);

    EXPECT_TRUE(matches(nfa, "a"));
}

TEST(MatchesNfa, HandlesSymbolsAboveAscii) {
    const char high = static_cast<char>(200);

    Nfa nfa;
    const StateId start = nfa.add_state();
    const StateId accept = nfa.add_state();
    nfa.add_start(start);
    nfa.add_transition(start, to_symbol(high), accept);
    nfa.set_accepting(accept);

    EXPECT_TRUE(matches(nfa, std::string(1, high)));
    EXPECT_FALSE(matches(nfa, "a"));
}

TEST(MatchesNfa, AnUnknownSymbolStopsTheRun) {
    const Nfa nfa = regex::to_nfa(*regex::parse("a*"));

    EXPECT_FALSE(nfa.alphabet().contains(to_symbol('z')));
    EXPECT_FALSE(matches(nfa, "az"));
    EXPECT_FALSE(matches(nfa, "za"));
}

TEST(MatchesNfa, SurvivesALongInput) {
    const Nfa nfa = regex::to_nfa(*regex::parse("(a*)*"));

    EXPECT_TRUE(matches(nfa, std::string(1000, 'a')));
    EXPECT_FALSE(matches(nfa, std::string(1000, 'a') + 'b'));
}

TEST(MatchesNfa, TakesAnEmbeddedZero) {
    Nfa nfa;
    const StateId start = nfa.add_state();
    const StateId accept = nfa.add_state();
    nfa.add_start(start);
    nfa.add_transition(start, to_symbol('\0'), accept);
    nfa.set_accepting(accept);

    EXPECT_TRUE(matches(nfa, std::string_view("\0", 1)));
    EXPECT_FALSE(matches(nfa, ""));
}

} // namespace
} // namespace formal::automata
