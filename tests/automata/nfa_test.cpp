#include <cstddef>

#include <gtest/gtest.h>

#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr std::size_t count_states(std::size_t how_many) {
    Nfa nfa;
    for (std::size_t i = 0; i < how_many; ++i) {
        (void)nfa.add_state();
    }
    return nfa.size();
}

static_assert(count_states(0) == 0);
static_assert(count_states(3) == 3);

TEST(Nfa, StartsWithoutStates) {
    const Nfa nfa;
    EXPECT_EQ(nfa.size(), 0U);
}

TEST(Nfa, NumbersStatesFromZero) {
    Nfa nfa;
    EXPECT_EQ(nfa.add_state(), StateId{0});
    EXPECT_EQ(nfa.add_state(), StateId{1});
    EXPECT_EQ(nfa.size(), 2U);
}

TEST(Nfa, KeepsNumbersDense) {
    Nfa nfa;
    for (StateId expected = 0; expected < 10; ++expected) {
        EXPECT_EQ(nfa.add_state(), expected);
    }
    EXPECT_EQ(nfa.size(), 10U);
}

constexpr Nfa two_states() {
    Nfa nfa;
    (void)nfa.add_state();
    (void)nfa.add_state();
    return nfa;
}

constexpr std::size_t transition_count(Symbol symbol, std::size_t how_many) {
    Nfa nfa = two_states();
    for (std::size_t i = 0; i < how_many; ++i) {
        nfa.add_transition(0, symbol, 1);
    }
    return nfa.transitions(0).size();
}

static_assert(transition_count(to_symbol('a'), 1) == 1);
static_assert(transition_count(to_symbol('a'), 5) == 1);

TEST(NfaTransitions, StartEmpty) {
    const Nfa nfa = two_states();
    EXPECT_TRUE(nfa.transitions(0).empty());
    EXPECT_TRUE(nfa.transitions(1).empty());
}

TEST(NfaTransitions, BelongToTheSourceStateOnly) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);

    ASSERT_EQ(nfa.transitions(0).size(), 1U);
    EXPECT_EQ(nfa.transitions(0)[0].symbol, to_symbol('a'));
    EXPECT_EQ(nfa.transitions(0)[0].target, StateId{1});
    EXPECT_TRUE(nfa.transitions(1).empty());
}

TEST(NfaTransitions, IgnoreDuplicates) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_EQ(nfa.transitions(0).size(), 1U);
}

TEST(NfaTransitions, KeepTheSameSymbolToDifferentTargets) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(0, to_symbol('a'), 0);

    EXPECT_EQ(nfa.transitions(0).size(), 2U);
}

// Порядок полей в Transition задаёт порядок сравнения: сначала символ.
TEST(NfaTransitions, AreSortedBySymbolThenTarget) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('b'), 1);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(0, to_symbol('a'), 0);

    const auto transitions = nfa.transitions(0);
    ASSERT_EQ(transitions.size(), 3U);
    EXPECT_EQ(transitions[0], (Nfa::Transition{to_symbol('a'), 0}));
    EXPECT_EQ(transitions[1], (Nfa::Transition{to_symbol('a'), 1}));
    EXPECT_EQ(transitions[2], (Nfa::Transition{to_symbol('b'), 1}));
}

} // namespace
} // namespace formal::automata
