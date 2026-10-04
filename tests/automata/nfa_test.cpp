#include <cstddef>

#include <gtest/gtest.h>

#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/alphabet.hpp"
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

constexpr std::size_t epsilon_count(std::size_t how_many) {
    Nfa nfa = two_states();
    for (std::size_t i = 0; i < how_many; ++i) {
        nfa.add_epsilon(0, 1);
    }
    return nfa.epsilons(0).size();
}

static_assert(epsilon_count(1) == 1);
static_assert(epsilon_count(5) == 1);

TEST(NfaEpsilons, StartEmpty) {
    const Nfa nfa = two_states();
    EXPECT_TRUE(nfa.epsilons(0).empty());
    EXPECT_TRUE(nfa.epsilons(1).empty());
}

TEST(NfaEpsilons, BelongToTheSourceStateOnly) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 1);

    ASSERT_EQ(nfa.epsilons(0).size(), 1U);
    EXPECT_EQ(nfa.epsilons(0)[0], StateId{1});
    EXPECT_TRUE(nfa.epsilons(1).empty());
}

TEST(NfaEpsilons, IgnoreDuplicates) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 1);
    nfa.add_epsilon(0, 1);

    EXPECT_EQ(nfa.epsilons(0).size(), 1U);
}

TEST(NfaEpsilons, AllowASelfLoop) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 0);

    ASSERT_EQ(nfa.epsilons(0).size(), 1U);
    EXPECT_EQ(nfa.epsilons(0)[0], StateId{0});
}

TEST(NfaEpsilons, DoNotMixWithLabelledTransitions) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_epsilon(0, 1);

    EXPECT_EQ(nfa.transitions(0).size(), 1U);
    EXPECT_EQ(nfa.epsilons(0).size(), 1U);
}

TEST(NfaEpsilons, AreSortedRegardlessOfInsertionOrder) {
    Nfa nfa = two_states();
    (void)nfa.add_state();
    nfa.add_epsilon(0, 2);
    nfa.add_epsilon(0, 0);
    nfa.add_epsilon(0, 1);

    const auto epsilons = nfa.epsilons(0);
    ASSERT_EQ(epsilons.size(), 3U);
    EXPECT_EQ(epsilons[0], StateId{0});
    EXPECT_EQ(epsilons[1], StateId{1});
    EXPECT_EQ(epsilons[2], StateId{2});
}

constexpr Alphabet alphabet_of(Symbol symbol) {
    Nfa nfa = two_states();
    nfa.add_transition(0, symbol, 1);
    return nfa.alphabet();
}

static_assert(Nfa{}.alphabet() == Alphabet{});
static_assert(alphabet_of(to_symbol('a')) == Alphabet::from_symbols("a"));

TEST(NfaAlphabet, StartsEmpty) {
    EXPECT_EQ(Nfa{}.alphabet(), Alphabet{});
}

TEST(NfaAlphabet, GrowsWithEveryTransition) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(1, to_symbol('b'), 0);

    EXPECT_EQ(nfa.alphabet(), Alphabet::from_symbols("ab"));
}

TEST(NfaAlphabet, CountsASymbolOnce) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(1, to_symbol('a'), 0);

    EXPECT_EQ(nfa.alphabet().size(), 1U);
}

TEST(NfaAlphabet, IgnoresEpsilonTransitions) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 1);

    EXPECT_EQ(nfa.alphabet(), Alphabet{});
}

TEST(NfaAlphabet, WidensWithSymbolsThatHaveNoTransition) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.widen_alphabet(Alphabet::from_symbols("bc"));

    EXPECT_EQ(nfa.alphabet(), Alphabet::from_symbols("abc"));
}

TEST(NfaAlphabet, WideningKeepsTheSymbolsOfTransitions) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.widen_alphabet(Alphabet{});

    EXPECT_EQ(nfa.alphabet(), Alphabet::from_symbols("a"));
}

TEST(NfaAlphabet, HandsOutACopy) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);

    Alphabet copy = nfa.alphabet();
    copy.add(to_symbol('z'));

    EXPECT_EQ(nfa.alphabet(), Alphabet::from_symbols("a"));
}

} // namespace
} // namespace formal::automata
