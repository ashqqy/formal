#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "automata/dfa.hpp"
#include "automata/state.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr Alphabet two = Alphabet::from_symbols("ac");

constexpr Dfa built() {
    Dfa dfa(two);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();
    dfa.set_transition(first, to_symbol('a'), second);
    dfa.set_transition(second, to_symbol('c'), second);
    return dfa;
}

static_assert(built().size() == 2);
static_assert(built().transition(0, to_symbol('a')) == 1);
static_assert(built().transition(1, to_symbol('c')) == 1);
static_assert(built().transition(0, to_symbol('c')) == kNoState);
static_assert(built().transition(1, to_symbol('a')) == kNoState);
static_assert(built().transition(0, to_symbol('z')) == kNoState);
static_assert(built().alphabet() == two);

constexpr Dfa marked() {
    Dfa dfa = built();
    dfa.set_start(0);
    dfa.set_accepting(1);
    return dfa;
}

static_assert(Dfa{two}.start() == kNoState);
static_assert(marked().start() == 0);
static_assert(!marked().is_accepting(0));
static_assert(marked().is_accepting(1));
static_assert(marked() == marked());
static_assert(!(marked() == built()));

TEST(Dfa, StartsWithoutStates) {
    const Dfa dfa(two);
    EXPECT_EQ(dfa.size(), 0U);
}

TEST(Dfa, KeepsTheAlphabetItWasGiven) {
    const Dfa dfa(Alphabet::from_symbols("ACGT"));
    EXPECT_EQ(dfa.alphabet(), Alphabet::from_symbols("ACGT"));
}

TEST(Dfa, NumbersStatesFromZero) {
    Dfa dfa(two);
    EXPECT_EQ(dfa.add_state(), 0U);
    EXPECT_EQ(dfa.add_state(), 1U);
    EXPECT_EQ(dfa.add_state(), 2U);
    EXPECT_EQ(dfa.size(), 3U);
}

TEST(Dfa, ANewStateHasNoTransitions) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();

    EXPECT_EQ(dfa.transition(state, to_symbol('a')), kNoState);
    EXPECT_EQ(dfa.transition(state, to_symbol('c')), kNoState);
}

TEST(Dfa, RemembersWhatWasSet) {
    Dfa dfa(two);
    const StateId from = dfa.add_state();
    const StateId to = dfa.add_state();
    dfa.set_transition(from, to_symbol('a'), to);

    EXPECT_EQ(dfa.transition(from, to_symbol('a')), to);
}

TEST(Dfa, OverwritesInsteadOfAdding) {
    Dfa dfa(two);
    const StateId from = dfa.add_state();
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();

    dfa.set_transition(from, to_symbol('a'), first);
    dfa.set_transition(from, to_symbol('a'), second);

    EXPECT_EQ(dfa.transition(from, to_symbol('a')), second);
}

TEST(Dfa, KeepsSymbolsApart) {
    Dfa dfa(two);
    const StateId from = dfa.add_state();
    const StateId to = dfa.add_state();
    dfa.set_transition(from, to_symbol('a'), to);

    EXPECT_EQ(dfa.transition(from, to_symbol('a')), to);
    EXPECT_EQ(dfa.transition(from, to_symbol('c')), kNoState);
}

TEST(Dfa, KeepsStatesApart) {
    Dfa dfa(two);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();
    dfa.set_transition(second, to_symbol('a'), first);

    EXPECT_EQ(dfa.transition(first, to_symbol('a')), kNoState);
    EXPECT_EQ(dfa.transition(second, to_symbol('a')), first);
}

TEST(Dfa, TakesASelfLoop) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();
    dfa.set_transition(state, to_symbol('a'), state);

    EXPECT_EQ(dfa.transition(state, to_symbol('a')), state);
}

TEST(Dfa, AddressesEveryCellOfTheTable) {
    const Alphabet alphabet = Alphabet::from_symbols("ace");

    Dfa dfa(alphabet);
    for (std::size_t i = 0; i < 3; ++i) {
        (void)dfa.add_state();
    }

    StateId target = 0;
    for (StateId state = 0; state < dfa.size(); ++state) {
        for (const Symbol symbol : alphabet) {
            dfa.set_transition(state, symbol, target % 3);
            ++target;
        }
    }

    target = 0;
    for (StateId state = 0; state < dfa.size(); ++state) {
        for (const Symbol symbol : alphabet) {
            EXPECT_EQ(dfa.transition(state, symbol), target % 3)
                << "state " << state << ", symbol " << symbol;
            ++target;
        }
    }
}

TEST(Dfa, WorksOnAnAlphabetSpanningChunks) {
    const Alphabet alphabet =
        Alphabet::from_symbols("\x01\x3f\x40\x7f\x80\xff");

    Dfa dfa(alphabet);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();

    for (const Symbol symbol : alphabet) {
        dfa.set_transition(first, symbol, second);
    }
    for (const Symbol symbol : alphabet) {
        EXPECT_EQ(dfa.transition(first, symbol), second)
            << "symbol " << static_cast<unsigned>(symbol);
        EXPECT_EQ(dfa.transition(second, symbol), kNoState);
    }
}

TEST(Dfa, SymbolsOutsideTheAlphabetHaveNoTransition) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();

    EXPECT_FALSE(dfa.alphabet().contains(to_symbol('z')));
    EXPECT_EQ(dfa.transition(state, to_symbol('z')), kNoState);
    EXPECT_EQ(dfa.transition(state, Symbol{0}), kNoState);
    EXPECT_EQ(dfa.transition(state, Symbol{255}), kNoState);
}

TEST(Dfa, SurvivesAnEmptyAlphabet) {
    Dfa dfa{Alphabet{}};
    const StateId state = dfa.add_state();

    EXPECT_EQ(dfa.size(), 1U);
    EXPECT_EQ(dfa.transition(state, to_symbol('a')), kNoState);
}

TEST(Dfa, HoldsTheWholeAlphabet) {
    Dfa dfa(Alphabet::all());
    const StateId from = dfa.add_state();
    const StateId to = dfa.add_state();

    dfa.set_transition(from, Symbol{0}, to);
    dfa.set_transition(from, Symbol{255}, to);

    EXPECT_EQ(dfa.transition(from, Symbol{0}), to);
    EXPECT_EQ(dfa.transition(from, Symbol{255}), to);
    EXPECT_EQ(dfa.transition(from, Symbol{128}), kNoState);
}

TEST(DfaStart, IsUnsetUntilItIsSet) {
    Dfa dfa(two);
    EXPECT_EQ(dfa.start(), kNoState);

    const StateId state = dfa.add_state();
    EXPECT_EQ(dfa.start(), kNoState);

    dfa.set_start(state);
    EXPECT_EQ(dfa.start(), state);
}

TEST(DfaStart, CanBeMoved) {
    Dfa dfa(two);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();

    dfa.set_start(first);
    dfa.set_start(second);

    EXPECT_EQ(dfa.start(), second);
}

TEST(DfaAccepting, ANewStateIsNotAccepting) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();
    EXPECT_FALSE(dfa.is_accepting(state));
}

TEST(DfaAccepting, IsMarkedByDefault) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();
    dfa.set_accepting(state);

    EXPECT_TRUE(dfa.is_accepting(state));
}

TEST(DfaAccepting, CanBeCleared) {
    Dfa dfa(two);
    const StateId state = dfa.add_state();
    dfa.set_accepting(state);
    dfa.set_accepting(state, false);

    EXPECT_FALSE(dfa.is_accepting(state));
}

TEST(DfaAccepting, IsIndependentOfTheStart) {
    Dfa dfa(two);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();
    dfa.set_start(first);
    dfa.set_accepting(second);

    EXPECT_FALSE(dfa.is_accepting(first));
    EXPECT_TRUE(dfa.is_accepting(second));
}

TEST(DfaAccepting, GrowsWithTheStates) {
    Dfa dfa(two);
    for (std::size_t i = 0; i < 5; ++i) {
        const StateId state = dfa.add_state();
        EXPECT_FALSE(dfa.is_accepting(state));
    }

    dfa.set_accepting(4);
    EXPECT_TRUE(dfa.is_accepting(4));
    EXPECT_FALSE(dfa.is_accepting(3));
}

TEST(DfaAccepting, KeepsStatesApart) {
    Dfa dfa(two);
    const StateId first = dfa.add_state();
    const StateId second = dfa.add_state();
    dfa.set_accepting(first);

    EXPECT_TRUE(dfa.is_accepting(first));
    EXPECT_FALSE(dfa.is_accepting(second));
}

TEST(DfaEquality, ComparesEqualToItself) {
    const Dfa dfa = marked();
    EXPECT_EQ(dfa, dfa);
    EXPECT_EQ(marked(), marked());
}

TEST(DfaEquality, IgnoresTheOrderOfCalls) {
    Dfa first(two);
    (void)first.add_state();
    (void)first.add_state();
    first.set_accepting(1);
    first.set_transition(0, to_symbol('a'), 1);
    first.set_start(0);

    Dfa second(two);
    (void)second.add_state();
    (void)second.add_state();
    second.set_start(0);
    second.set_transition(0, to_symbol('a'), 1);
    second.set_accepting(1);

    EXPECT_EQ(first, second);
}

TEST(DfaEquality, NoticesADifferentAlphabet) {
    EXPECT_NE(Dfa{Alphabet::from_symbols("a")},
              Dfa{Alphabet::from_symbols("b")});
}

TEST(DfaEquality, NoticesADifferentStateCount) {
    Dfa one(two);
    (void)one.add_state();

    Dfa other(two);
    (void)other.add_state();
    (void)other.add_state();

    EXPECT_NE(one, other);
}

TEST(DfaEquality, NoticesADifferentTransition) {
    Dfa one = built();
    Dfa other = built();
    other.set_transition(0, to_symbol('a'), 0);

    EXPECT_NE(one, other);
}

TEST(DfaEquality, NoticesADifferentStart) {
    Dfa one = built();
    Dfa other = built();
    one.set_start(0);
    other.set_start(1);

    EXPECT_NE(one, other);
}

TEST(DfaEquality, NoticesADifferentAcceptingSet) {
    Dfa one = built();
    Dfa other = built();
    one.set_accepting(0);
    other.set_accepting(1);

    EXPECT_NE(one, other);
}

} // namespace
} // namespace formal::automata
