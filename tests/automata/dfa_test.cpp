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

} // namespace
} // namespace formal::automata
