#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "automata/closure.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "automata/step.hpp"
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

std::vector<StateId> step_of(const Nfa& nfa, std::vector<StateId> from,
                             char symbol) {
    return step(nfa, from, to_symbol(symbol));
}

constexpr std::size_t step_size(const char* pattern, char symbol) {
    const Nfa nfa = regex::to_nfa(*regex::parse(pattern));
    return step(nfa, epsilon_closure(nfa, nfa.starts()), to_symbol(symbol))
        .size();
}

static_assert(step_size("a", 'a') == 1);
static_assert(step_size("a", 'b') == 0);
static_assert(step_size("a|b", 'a') == 1);

TEST(Step, FromNothingIsNothing) {
    Nfa nfa = states(2);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_TRUE(step_of(nfa, {}, 'a').empty());
}

TEST(Step, CollectsEveryTargetOfTheSymbol) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(0, to_symbol('a'), 2);

    EXPECT_EQ(step_of(nfa, {0}, 'a'), (std::vector<StateId>{1, 2}));
}

TEST(Step, IgnoresOtherSymbols) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(0, to_symbol('b'), 2);

    EXPECT_EQ(step_of(nfa, {0}, 'a'), (std::vector<StateId>{1}));
    EXPECT_EQ(step_of(nfa, {0}, 'b'), (std::vector<StateId>{2}));
}

TEST(Step, OnAnUnknownSymbolIsEmpty) {
    Nfa nfa = states(2);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_FALSE(nfa.alphabet().contains(to_symbol('z')));
    EXPECT_TRUE(step_of(nfa, {0}, 'z').empty());
}

TEST(Step, IgnoresEpsilonEdges) {
    Nfa nfa = states(2);
    nfa.add_epsilon(0, 1);

    EXPECT_TRUE(step_of(nfa, {0}, 'a').empty());
}

TEST(Step, DoesNotCloseOverEpsilonAfterwards) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_epsilon(1, 2);

    EXPECT_EQ(step_of(nfa, {0}, 'a'), (std::vector<StateId>{1}));
}

TEST(Step, MergesSourcesWithoutDuplicates) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 2);
    nfa.add_transition(1, to_symbol('a'), 2);

    EXPECT_EQ(step_of(nfa, {0, 1}, 'a'), (std::vector<StateId>{2}));
}

TEST(Step, IgnoresRepeatsInTheInput) {
    Nfa nfa = states(2);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_EQ(step_of(nfa, {0, 0, 0}, 'a'), (std::vector<StateId>{1}));
}

TEST(Step, ReturnsStatesInOrder) {
    Nfa nfa = states(4);
    nfa.add_transition(0, to_symbol('a'), 3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(2, to_symbol('a'), 0);

    EXPECT_EQ(step_of(nfa, {2, 0}, 'a'), (std::vector<StateId>{0, 1, 3}));
}

TEST(Step, DoesNotDependOnTheOrderOfSources) {
    Nfa nfa = states(4);
    nfa.add_transition(0, to_symbol('a'), 3);
    nfa.add_transition(1, to_symbol('a'), 2);

    EXPECT_EQ(step_of(nfa, {0, 1}, 'a'), step_of(nfa, {1, 0}, 'a'));
}

TEST(Step, SeesOnlyTheGivenSources) {
    Nfa nfa = states(3);
    nfa.add_transition(0, to_symbol('a'), 1);
    nfa.add_transition(2, to_symbol('a'), 1);

    EXPECT_TRUE(step_of(nfa, {1}, 'a').empty());
}

TEST(Step, PairsWithTheClosureOnARealAutomaton) {
    const Nfa nfa = regex::to_nfa(*regex::parse("ab"));
    const std::vector<StateId> start = epsilon_closure(nfa, nfa.starts());

    EXPECT_TRUE(step(nfa, start, to_symbol('b')).empty());

    const std::vector<StateId> after_a =
        epsilon_closure(nfa, step(nfa, start, to_symbol('a')));
    EXPECT_FALSE(after_a.empty());

    const std::vector<StateId> after_b =
        epsilon_closure(nfa, step(nfa, after_a, to_symbol('b')));
    bool accepting = false;
    for (const StateId id : after_b) {
        accepting = accepting || nfa.is_accepting(id);
    }
    EXPECT_TRUE(accepting);
}

} // namespace
} // namespace formal::automata
