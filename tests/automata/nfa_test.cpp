#include <cstddef>

#include <gtest/gtest.h>

#include "automata/nfa.hpp"
#include "automata/state.hpp"

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

} // namespace
} // namespace formal::automata
