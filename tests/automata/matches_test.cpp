#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

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

constexpr Dfa star_of_a() {
    Dfa dfa{Alphabet::from_symbols("a")};
    const StateId only = dfa.add_state();
    dfa.set_start(only);
    dfa.set_accepting(only);
    dfa.set_transition(only, to_symbol('a'), only);
    return dfa;
}

constexpr Dfa literal_ab() {
    Dfa dfa{Alphabet::from_symbols("ab")};
    const StateId start = dfa.add_state();
    const StateId after_a = dfa.add_state();
    const StateId after_b = dfa.add_state();
    dfa.set_start(start);
    dfa.set_accepting(after_b);
    dfa.set_transition(start, to_symbol('a'), after_a);
    dfa.set_transition(after_a, to_symbol('b'), after_b);
    return dfa;
}

constexpr Dfa a_or_b_c_star() {
    Dfa dfa{Alphabet::from_symbols("abc")};
    const StateId start = dfa.add_state();
    const StateId after_a = dfa.add_state();
    const StateId after_b = dfa.add_state();
    dfa.set_start(start);
    dfa.set_accepting(after_a);
    dfa.set_accepting(after_b);
    dfa.set_transition(start, to_symbol('a'), after_a);
    dfa.set_transition(start, to_symbol('b'), after_b);
    dfa.set_transition(after_b, to_symbol('c'), after_b);
    return dfa;
}

static_assert(matches(star_of_a(), ""));
static_assert(matches(star_of_a(), "aaa"));
static_assert(!matches(star_of_a(), "b"));
static_assert(matches(literal_ab(), "ab"));
static_assert(!matches(literal_ab(), "a"));
static_assert(!matches(literal_ab(), "abb"));
static_assert(matches(a_or_b_c_star(), "bcc"));

TEST(MatchesDfa, TakesTheEmptyStringWhenTheStartAccepts) {
    EXPECT_TRUE(matches(star_of_a(), ""));
    EXPECT_FALSE(matches(literal_ab(), ""));
}

TEST(MatchesDfa, FollowsASelfLoop) {
    EXPECT_TRUE(matches(star_of_a(), "a"));
    EXPECT_TRUE(matches(star_of_a(), std::string(1000, 'a')));
}

TEST(MatchesDfa, WantsTheWholeInput) {
    EXPECT_TRUE(matches(literal_ab(), "ab"));
    EXPECT_FALSE(matches(literal_ab(), "a"));
    EXPECT_FALSE(matches(literal_ab(), "abb"));
    EXPECT_FALSE(matches(literal_ab(), "b"));
}

TEST(MatchesDfa, WithoutAStartAcceptsNothing) {
    Dfa dfa{Alphabet::from_symbols("a")};
    const StateId state = dfa.add_state();
    dfa.set_accepting(state);

    EXPECT_FALSE(matches(dfa, ""));
    EXPECT_FALSE(matches(dfa, "a"));
}

TEST(MatchesDfa, WithoutStatesAcceptsNothing) {
    const Dfa dfa{Alphabet::from_symbols("a")};
    EXPECT_FALSE(matches(dfa, ""));
    EXPECT_FALSE(matches(dfa, "a"));
}

TEST(MatchesDfa, StopsOnAHole) {
    EXPECT_FALSE(matches(literal_ab(), "aa"));
    EXPECT_FALSE(matches(literal_ab(), "ba"));
}

TEST(MatchesDfa, StopsOnASymbolOutsideTheAlphabet) {
    EXPECT_FALSE(matches(star_of_a(), "z"));
    EXPECT_FALSE(matches(star_of_a(), "az"));
    EXPECT_FALSE(matches(star_of_a(), "za"));
    EXPECT_FALSE(matches(a_or_b_c_star(), "bcz"));
}

TEST(MatchesDfa, HandlesSymbolsAboveAscii) {
    const char high = static_cast<char>(200);

    Dfa dfa{Alphabet::from_symbols(std::string(1, high))};
    const StateId start = dfa.add_state();
    const StateId accept = dfa.add_state();
    dfa.set_start(start);
    dfa.set_accepting(accept);
    dfa.set_transition(start, to_symbol(high), accept);

    EXPECT_TRUE(matches(dfa, std::string(1, high)));
    EXPECT_FALSE(matches(dfa, "a"));
}

TEST(MatchesDfa, TakesAnEmbeddedZero) {
    Dfa dfa{Alphabet::from_symbols(std::string_view("\0", 1))};
    const StateId start = dfa.add_state();
    const StateId accept = dfa.add_state();
    dfa.set_start(start);
    dfa.set_accepting(accept);
    dfa.set_transition(start, Symbol{0}, accept);

    EXPECT_TRUE(matches(dfa, std::string_view("\0", 1)));
    EXPECT_FALSE(matches(dfa, ""));
}

TEST(MatchesDfa, AgreesWithAnEquivalentNfa) {
    const Nfa nfa = regex::to_nfa(*regex::parse("a|bc*"));
    const Dfa dfa = a_or_b_c_star();

    for (const std::string_view input :
         {"", "a", "b", "c", "aa", "ab", "ba", "bc", "bcc", "bccc", "cb", "abc",
          "bca", "z", "bcz"}) {
        EXPECT_EQ(matches(dfa, input), matches(nfa, input))
            << "input \"" << input << '"';
    }
}

} // namespace
} // namespace formal::automata
