#include <cstddef>
#include <string_view>

#include <gtest/gtest.h>

#include "automata/complete.hpp"
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

static_assert(!is_complete(determinized("ab")));
static_assert(is_complete(complete(determinized("ab"))));
static_assert(complete(determinized("ab")).size() == 4);
static_assert(is_complete(determinized("a*")));
static_assert(complete(determinized("a*")) == determinized("a*"));

TEST(IsComplete, FindsAHole) {
    EXPECT_FALSE(is_complete(determinized("ab")));
    EXPECT_FALSE(is_complete(determinized("a|bc*")));
}

TEST(IsComplete, AcceptsAnAutomatonWithoutHoles) {
    EXPECT_TRUE(is_complete(determinized("a*")));
    EXPECT_TRUE(is_complete(determinized("(a|b)*")));
}

TEST(IsComplete, IsVacuouslyTrueWithoutSymbols) {
    Dfa dfa{Alphabet{}};
    (void)dfa.add_state();
    (void)dfa.add_state();

    EXPECT_TRUE(is_complete(dfa));
}

TEST(IsComplete, IsVacuouslyTrueWithoutStates) {
    EXPECT_TRUE(is_complete(Dfa{Alphabet::from_symbols("ab")}));
}

TEST(Complete, AddsExactlyOneState) {
    const Dfa dfa = determinized("ab");
    EXPECT_EQ(complete(dfa).size(), dfa.size() + 1);
}

TEST(Complete, LeavesACompleteAutomatonAlone) {
    const Dfa dfa = determinized("a*");
    EXPECT_EQ(complete(dfa), dfa);
    EXPECT_EQ(complete(dfa).size(), dfa.size());
}

TEST(Complete, FillsEveryHole) {
    const Dfa full = complete(determinized("a|bc*"));

    EXPECT_TRUE(is_complete(full));
    for (StateId id = 0; id < full.size(); ++id) {
        for (const Symbol symbol : full.alphabet()) {
            EXPECT_NE(full.transition(id, symbol), kNoState);
        }
    }
}

TEST(Complete, IsIdempotent) {
    const Dfa once = complete(determinized("ab"));
    EXPECT_EQ(complete(once), once);
}

TEST(Complete, KeepsTheStartAndTheAcceptingStates) {
    const Dfa dfa = determinized("ab");
    const Dfa full = complete(dfa);

    EXPECT_EQ(full.start(), dfa.start());
    for (StateId id = 0; id < dfa.size(); ++id) {
        EXPECT_EQ(full.is_accepting(id), dfa.is_accepting(id));
    }
}

TEST(Complete, SinkIsNotAccepting) {
    const Dfa full = complete(determinized("ab"));
    const StateId sink = to_state_id(full.size() - 1);

    EXPECT_FALSE(full.is_accepting(sink));
}

TEST(Complete, SinkLoopsOnEverySymbol) {
    const Dfa full = complete(determinized("ab"));
    const StateId sink = to_state_id(full.size() - 1);

    for (const Symbol symbol : full.alphabet()) {
        EXPECT_EQ(full.transition(sink, symbol), sink);
    }
}

TEST(Complete, AddsNoSinkWithoutSymbols) {
    Dfa dfa{Alphabet{}};
    const StateId state = dfa.add_state();
    dfa.set_start(state);

    EXPECT_EQ(complete(dfa), dfa);
}

TEST(Complete, KeepsTheLanguage) {
    constexpr std::string_view inputs[] = {
        "",   "a",   "b",    "c",   "aa",  "ab",  "ba", "bc",
        "cc", "bcc", "abab", "abc", "aaa", "bca", "cb", "z"};

    for (const char* pattern :
         {"a", "a*", "ab", "abc", "(ab)*", "a|b", "a|b|c", "(a|b)*a", "a|bc*",
          "(a|b)*c", "()", "()*", "(a|)"}) {
        const Nfa nfa = regex::to_nfa(*regex::parse(pattern));
        const Dfa dfa = determinize(nfa);
        const Dfa full = complete(dfa);

        for (const std::string_view input : inputs) {
            EXPECT_EQ(matches(full, input), matches(dfa, input))
                << "pattern " << pattern << ", input \"" << input << '"';
            EXPECT_EQ(matches(full, input), matches(nfa, input))
                << "pattern " << pattern << ", input \"" << input << '"';
        }
    }
}

} // namespace
} // namespace formal::automata
