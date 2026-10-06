#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "automata/closure.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/parser.hpp"
#include "regex/thompson.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

Nfa states(std::size_t how_many) {
    Nfa nfa;
    for (std::size_t i = 0; i < how_many; ++i) {
        (void)nfa.add_state();
    }
    return nfa;
}

std::vector<StateId> closure_of(const Nfa& nfa, std::vector<StateId> from) {
    return epsilon_closure(nfa, from);
}

constexpr std::size_t closure_size(const char* pattern) {
    const Nfa nfa = regex::to_nfa(*regex::parse(pattern));
    return epsilon_closure(nfa, nfa.starts()).size();
}

static_assert(closure_size("a") == 1);
static_assert(closure_size("a*") == 3);
static_assert(closure_size("(a*)*") == 5);

TEST(EpsilonClosure, OfNothingIsNothing) {
    const Nfa nfa = states(2);
    EXPECT_TRUE(closure_of(nfa, {}).empty());
}

TEST(EpsilonClosure, WithoutEpsilonEdgesReturnsTheInput) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_EQ(closure_of(nfa, {0}), (std::vector<StateId>{0}));
    EXPECT_EQ(closure_of(nfa, {0, 2}), (std::vector<StateId>{0, 2}));
}

TEST(EpsilonClosure, FollowsAChain) {
    Nfa nfa = states(3);
    nfa.add_epsilon(0, 1);
    nfa.add_epsilon(1, 2);

    EXPECT_EQ(closure_of(nfa, {0}), (std::vector<StateId>{0, 1, 2}));
    EXPECT_EQ(closure_of(nfa, {1}), (std::vector<StateId>{1, 2}));
    EXPECT_EQ(closure_of(nfa, {2}), (std::vector<StateId>{2}));
}

TEST(EpsilonClosure, DoesNotFollowLabelledTransitions) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_epsilon(1, 2);

    EXPECT_EQ(closure_of(nfa, {0}), (std::vector<StateId>{0}));
}

TEST(EpsilonClosure, TerminatesOnASelfLoop) {
    Nfa nfa = states(2);
    nfa.add_epsilon(0, 0);

    EXPECT_EQ(closure_of(nfa, {0}), (std::vector<StateId>{0}));
}

TEST(EpsilonClosure, TerminatesOnACycle) {
    Nfa nfa = states(3);
    nfa.add_epsilon(0, 1);
    nfa.add_epsilon(1, 2);
    nfa.add_epsilon(2, 0);

    EXPECT_EQ(closure_of(nfa, {0}), (std::vector<StateId>{0, 1, 2}));
    EXPECT_EQ(closure_of(nfa, {1}), (std::vector<StateId>{0, 1, 2}));
}

TEST(EpsilonClosure, MergesSeveralSources) {
    Nfa nfa = states(4);
    nfa.add_epsilon(0, 1);
    nfa.add_epsilon(2, 3);

    EXPECT_EQ(closure_of(nfa, {0, 2}), (std::vector<StateId>{0, 1, 2, 3}));
}

TEST(EpsilonClosure, IgnoresRepeatsInTheInput) {
    Nfa nfa = states(2);
    nfa.add_epsilon(0, 1);

    EXPECT_EQ(closure_of(nfa, {0, 0, 0}), (std::vector<StateId>{0, 1}));
}

TEST(EpsilonClosure, ReturnsStatesInOrder) {
    Nfa nfa = states(4);
    nfa.add_epsilon(3, 1);
    nfa.add_epsilon(1, 0);

    EXPECT_EQ(closure_of(nfa, {3}), (std::vector<StateId>{0, 1, 3}));
}

TEST(EpsilonClosure, ReachesWhatAStarOffersForFree) {
    const Nfa nfa = regex::to_nfa(*regex::parse("a*"));
    const std::vector<StateId> start = epsilon_closure(nfa, nfa.starts());

    EXPECT_EQ(start.size(), 3U);
    bool accepting = false;
    for (const StateId id : start) {
        accepting = accepting || nfa.is_accepting(id);
    }
    EXPECT_TRUE(accepting);
}

TEST(EpsilonClosure, TerminatesOnNestedStars) {
    const Nfa nfa = regex::to_nfa(*regex::parse("((a*)*)*"));
    EXPECT_EQ(epsilon_closure(nfa, nfa.starts()).size(), 7U);
}

} // namespace
} // namespace formal::automata
