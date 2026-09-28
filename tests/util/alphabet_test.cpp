#include <initializer_list>
#include <stdexcept>

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

static_assert(Alphabet{}.size() == 0);
static_assert(made({to_symbol('a'), to_symbol('a')}).size() == 1);
static_assert(made({to_symbol('a'), Symbol{200}}).size() == 2);

constexpr Alphabet dna = Alphabet::from_symbols("ACGT");
constexpr Alphabet digits =
    Alphabet::from_range(to_symbol('0'), to_symbol('9'));
constexpr Alphabet everything = Alphabet::all();

static_assert(dna.size() == 4);
static_assert(dna.contains(to_symbol('A')));
static_assert(!dna.contains(to_symbol('B')));
static_assert(digits.size() == 10);
static_assert(everything.size() == 256);

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

TEST(AlphabetSize, CountsNothingWhenEmpty) {
    EXPECT_EQ(Alphabet{}.size(), 0U);
}

TEST(AlphabetSize, CountsDistinctSymbols) {
    const Alphabet alphabet =
        made({to_symbol('a'), to_symbol('b'), to_symbol('c')});
    EXPECT_EQ(alphabet.size(), 3U);
}

TEST(AlphabetSize, IgnoresRepeats) {
    EXPECT_EQ(made({to_symbol('a'), to_symbol('a')}).size(), 1U);
}

TEST(AlphabetSize, CountsAcrossChunks) {
    const Alphabet alphabet =
        made({Symbol{0}, to_symbol('a'), Symbol{200}, Symbol{255}});
    EXPECT_EQ(alphabet.size(), 4U);
}

// Fills every chunk, so a chunk lost while summing shows up here.
TEST(AlphabetSize, CountsEverySymbol) {
    Alphabet alphabet;
    for (int value = 0; value < 256; ++value) {
        alphabet.add(static_cast<Symbol>(value));
    }
    EXPECT_EQ(alphabet.size(), 256U);
}

TEST(AlphabetFromSymbols, TakesEverySymbolOfTheString) {
    EXPECT_EQ(dna.size(), 4U);
    EXPECT_TRUE(dna.contains(to_symbol('A')));
    EXPECT_TRUE(dna.contains(to_symbol('T')));
    EXPECT_FALSE(dna.contains(to_symbol('B')));
}

TEST(AlphabetFromSymbols, IgnoresRepeats) {
    EXPECT_EQ(Alphabet::from_symbols("aa").size(), 1U);
}

TEST(AlphabetFromSymbols, EmptyStringGivesAnEmptyAlphabet) {
    EXPECT_EQ(Alphabet::from_symbols(""), Alphabet{});
}

// Both bounds belong to the range: '0'..'9' is ten symbols, not nine.
TEST(AlphabetFromRange, IncludesBothBounds) {
    EXPECT_EQ(digits.size(), 10U);
    EXPECT_TRUE(digits.contains(to_symbol('0')));
    EXPECT_TRUE(digits.contains(to_symbol('9')));
    EXPECT_FALSE(digits.contains(to_symbol('/')));
    EXPECT_FALSE(digits.contains(to_symbol(':')));
}

TEST(AlphabetFromRange, AcceptsASingleSymbol) {
    EXPECT_EQ(Alphabet::from_range(to_symbol('a'), to_symbol('a')).size(), 1U);
}

// The loop counter must outgrow Symbol, or 255 never ends it.
TEST(AlphabetFromRange, ReachesTheLastSymbol) {
    EXPECT_EQ(Alphabet::from_range(Symbol{250}, Symbol{255}).size(), 6U);
    EXPECT_EQ(Alphabet::from_range(Symbol{0}, Symbol{255}), Alphabet::all());
}

TEST(AlphabetFromRange, RejectsInvertedBounds) {
    EXPECT_THROW((void)Alphabet::from_range(to_symbol('9'), to_symbol('0')),
                 std::invalid_argument);
}

TEST(AlphabetAll, HoldsEverySymbol) {
    EXPECT_EQ(everything.size(), 256U);
    EXPECT_TRUE(everything.contains(Symbol{0}));
    EXPECT_TRUE(everything.contains(Symbol{255}));
    EXPECT_NE(everything, Alphabet{});
}

} // namespace
} // namespace formal
