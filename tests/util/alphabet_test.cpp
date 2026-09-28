#include <initializer_list>

#include <gtest/gtest.h>

#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal {
namespace {

constexpr Alphabet made(std::initializer_list<Symbol> symbols) {
    Alphabet alphabet;
    for (const Symbol symbol : symbols) {
        alphabet.add(symbol);
    }
    return alphabet;
}

static_assert(!Alphabet{}.contains(to_symbol('a')));
static_assert(made({to_symbol('a')}).contains(to_symbol('a')));
static_assert(!made({to_symbol('a')}).contains(to_symbol('b')));
static_assert(made({Symbol{0}, Symbol{255}}).contains(Symbol{255}));

TEST(Alphabet, StartsEmpty) {
    const Alphabet alphabet;
    EXPECT_FALSE(alphabet.contains(to_symbol('a')));
    EXPECT_FALSE(alphabet.contains(Symbol{0}));
    EXPECT_FALSE(alphabet.contains(Symbol{255}));
}

// 'b' and 'A' sit in the same chunk as 'a'.
TEST(Alphabet, HoldsOnlyWhatWasAdded) {
    const Alphabet alphabet = made({to_symbol('a')});
    EXPECT_TRUE(alphabet.contains(to_symbol('a')));
    EXPECT_FALSE(alphabet.contains(to_symbol('b')));
    EXPECT_FALSE(alphabet.contains(to_symbol('A')));
}

TEST(Alphabet, AddingTwiceChangesNothing) {
    Alphabet alphabet = made({to_symbol('a')});
    alphabet.add(to_symbol('a'));
    EXPECT_EQ(alphabet, made({to_symbol('a')}));
}

// 'a' is bit 33 of chunk 1, 200 is bit 8 of chunk 3.
TEST(Alphabet, KeepsChunksApart) {
    const Alphabet alphabet = made({to_symbol('a'), Symbol{200}});
    EXPECT_TRUE(alphabet.contains(to_symbol('a')));
    EXPECT_TRUE(alphabet.contains(Symbol{200}));
    EXPECT_FALSE(alphabet.contains(Symbol{8}));
    EXPECT_FALSE(alphabet.contains(Symbol{33}));
}

TEST(Alphabet, CoversTheWholeRange) {
    const Alphabet alphabet = made({Symbol{0}, Symbol{255}});
    EXPECT_TRUE(alphabet.contains(Symbol{0}));
    EXPECT_TRUE(alphabet.contains(Symbol{255}));
    EXPECT_FALSE(alphabet.contains(Symbol{1}));
    EXPECT_FALSE(alphabet.contains(Symbol{254}));
}

TEST(Alphabet, ComparesByContent) {
    EXPECT_EQ(made({to_symbol('a'), to_symbol('b')}),
              made({to_symbol('b'), to_symbol('a')}));
    EXPECT_NE(made({to_symbol('a')}), made({to_symbol('b')}));
    EXPECT_NE(made({to_symbol('a')}), Alphabet{});
}

} // namespace
} // namespace formal
