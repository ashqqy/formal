#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

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

constexpr Alphabet merged() {
    Alphabet alphabet = Alphabet::from_symbols("ab");
    alphabet.merge(Alphabet::from_symbols("bc"));
    return alphabet;
}

static_assert(merged() == Alphabet::from_symbols("abc"));

static_assert(dna.index_of(to_symbol('A')) == 0);
static_assert(dna.index_of(to_symbol('C')) == 1);
static_assert(dna.index_of(to_symbol('G')) == 2);
static_assert(dna.index_of(to_symbol('T')) == 3);
static_assert(everything.index_of(Symbol{200}) == 200);

constexpr bool indexes_agree_with_a_scan(const Alphabet& alphabet) {
    std::size_t expected = 0;
    for (std::size_t value = 0; value < 256; ++value) {
        const Symbol symbol = static_cast<Symbol>(value);
        if (!alphabet.contains(symbol)) { continue; }
        if (alphabet.index_of(symbol) != expected) { return false; }
        ++expected;
    }
    return expected == alphabet.size();
}

static_assert(indexes_agree_with_a_scan(dna));
static_assert(indexes_agree_with_a_scan(digits));
static_assert(indexes_agree_with_a_scan(everything));
static_assert(indexes_agree_with_a_scan(
    made({Symbol{0}, Symbol{63}, Symbol{64}, Symbol{65}, Symbol{127},
          Symbol{128}, Symbol{191}, Symbol{192}, Symbol{255}})));

static_assert(std::forward_iterator<Alphabet::Iterator>);
static_assert(std::sentinel_for<Alphabet::Iterator, Alphabet::Iterator>);
static_assert(std::ranges::forward_range<Alphabet>);
static_assert(std::regular<Alphabet::Iterator>);
static_assert(std::is_trivially_copyable_v<Alphabet::Iterator>);
static_assert(
    std::is_void_v<std::iterator_traits<Alphabet::Iterator>::pointer>);
static_assert(std::same_as<std::iter_value_t<Alphabet::Iterator>, Symbol>);

constexpr std::size_t walked(const Alphabet& alphabet) {
    std::size_t count = 0;
    for ([[maybe_unused]] const Symbol symbol : alphabet) {
        ++count;
    }
    return count;
}

constexpr bool walk_is_ascending_and_dense(const Alphabet& alphabet) {
    std::size_t position = 0;
    Symbol previous = 0;
    for (const Symbol symbol : alphabet) {
        if (position != 0 && symbol <= previous) { return false; }
        if (alphabet.index_of(symbol) != position) { return false; }
        previous = symbol;
        ++position;
    }
    return position == alphabet.size();
}

static_assert(walked(Alphabet{}) == 0);
static_assert(walked(dna) == 4);
static_assert(walked(digits) == 10);
static_assert(walked(everything) == 256);
static_assert(walk_is_ascending_and_dense(dna));
static_assert(walk_is_ascending_and_dense(digits));
static_assert(walk_is_ascending_and_dense(everything));
static_assert(walk_is_ascending_and_dense(made({Symbol{0}, Symbol{63},
                                                Symbol{64}, Symbol{255}})));

TEST(Alphabet, StartsEmpty) {
    const Alphabet alphabet;
    EXPECT_FALSE(alphabet.contains(to_symbol('a')));
    EXPECT_FALSE(alphabet.contains(Symbol{0}));
    EXPECT_FALSE(alphabet.contains(Symbol{255}));
}

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

TEST(AlphabetMerge, AddsSymbolsOfTheOther) {
    Alphabet alphabet = Alphabet::from_symbols("ab");
    alphabet.merge(Alphabet::from_symbols("cd"));

    EXPECT_EQ(alphabet, Alphabet::from_symbols("abcd"));
}

TEST(AlphabetMerge, KeepsWhatWasAlreadyThere) {
    Alphabet alphabet = Alphabet::from_symbols("ab");
    alphabet.merge(Alphabet::from_symbols("bc"));

    EXPECT_EQ(alphabet, Alphabet::from_symbols("abc"));
}

TEST(AlphabetMerge, WithAnEmptyAlphabetChangesNothing) {
    Alphabet alphabet = Alphabet::from_symbols("ab");
    alphabet.merge(Alphabet{});

    EXPECT_EQ(alphabet, Alphabet::from_symbols("ab"));
}

TEST(AlphabetMerge, ReachesEveryChunk) {
    Alphabet alphabet = Alphabet::from_symbols("a");
    alphabet.merge(made({Symbol{0}, Symbol{200}, Symbol{255}}));

    EXPECT_EQ(alphabet.size(), 4U);
    EXPECT_TRUE(alphabet.contains(Symbol{0}));
    EXPECT_TRUE(alphabet.contains(Symbol{255}));
}

TEST(AlphabetIndexOf, NumbersSymbolsFromZero) {
    EXPECT_EQ(dna.index_of(to_symbol('A')), 0U);
    EXPECT_EQ(dna.index_of(to_symbol('C')), 1U);
    EXPECT_EQ(dna.index_of(to_symbol('G')), 2U);
    EXPECT_EQ(dna.index_of(to_symbol('T')), 3U);
}

TEST(AlphabetIndexOf, IsDenseOverARange) {
    for (Symbol digit = to_symbol('0'); digit <= to_symbol('9'); ++digit) {
        EXPECT_EQ(digits.index_of(digit),
                  static_cast<std::size_t>(digit - to_symbol('0')));
    }
}

TEST(AlphabetIndexOf, OverEverythingIsTheSymbolItself) {
    for (std::size_t value = 0; value < 256; ++value) {
        const Symbol symbol = static_cast<Symbol>(value);
        EXPECT_EQ(everything.index_of(symbol), value);
    }
}

TEST(AlphabetIndexOf, AgreesWithAnAscendingScan) {
    EXPECT_TRUE(indexes_agree_with_a_scan(dna));
    EXPECT_TRUE(indexes_agree_with_a_scan(digits));
    EXPECT_TRUE(indexes_agree_with_a_scan(everything));
    EXPECT_TRUE(indexes_agree_with_a_scan(Alphabet::from_symbols("zyx")));
}

TEST(AlphabetIndexOf, CrossesChunkBoundaries) {
    const Alphabet alphabet = made({Symbol{63}, Symbol{64}, Symbol{255}});

    EXPECT_EQ(alphabet.index_of(Symbol{63}), 0U);
    EXPECT_EQ(alphabet.index_of(Symbol{64}), 1U);
    EXPECT_EQ(alphabet.index_of(Symbol{255}), 2U);
}

TEST(AlphabetIndexOf, CountsFullChunksBelow) {
    Alphabet alphabet = Alphabet::from_range(Symbol{0}, Symbol{127});
    alphabet.add(Symbol{200});

    EXPECT_EQ(alphabet.index_of(Symbol{200}), 128U);
}

TEST(AlphabetIndexOf, NumbersTheOnlySymbolZero) {
    EXPECT_EQ(made({Symbol{255}}).index_of(Symbol{255}), 0U);
    EXPECT_EQ(made({Symbol{0}}).index_of(Symbol{0}), 0U);
}

TEST(AlphabetIndexOf, NumbersTheLastSymbolJustBelowTheSize) {
    EXPECT_EQ(dna.index_of(to_symbol('T')), dna.size() - 1);
    EXPECT_EQ(digits.index_of(to_symbol('9')), digits.size() - 1);
    EXPECT_EQ(everything.index_of(Symbol{255}), everything.size() - 1);
}

TEST(AlphabetIndexOf, IgnoresSymbolsAbove) {
    const Alphabet alphabet = made({to_symbol('a'), to_symbol('b')});
    EXPECT_EQ(alphabet.index_of(to_symbol('a')), 0U);
}

std::vector<Symbol> collected(const Alphabet& alphabet) {
    return {alphabet.begin(), alphabet.end()};
}

TEST(AlphabetIterator, OverAnEmptyAlphabetStopsAtOnce) {
    const Alphabet alphabet;
    EXPECT_EQ(alphabet.begin(), alphabet.end());
    EXPECT_TRUE(collected(alphabet).empty());
}

TEST(AlphabetIterator, VisitsEverySymbolInAscendingOrder) {
    EXPECT_EQ(collected(dna),
              (std::vector<Symbol>{to_symbol('A'), to_symbol('C'),
                                   to_symbol('G'), to_symbol('T')}));
}

TEST(AlphabetIterator, DoesNotFollowTheOrderOfInsertion) {
    EXPECT_EQ(collected(Alphabet::from_symbols("TGCA")), collected(dna));
}

TEST(AlphabetIterator, CrossesChunkBoundaries) {
    const Alphabet alphabet = made({Symbol{63}, Symbol{64}, Symbol{255}});
    EXPECT_EQ(collected(alphabet),
              (std::vector<Symbol>{Symbol{63}, Symbol{64}, Symbol{255}}));
}

TEST(AlphabetIterator, ReachesBothEndsOfTheRange) {
    const std::vector<Symbol> all = collected(everything);

    ASSERT_EQ(all.size(), 256U);
    EXPECT_EQ(all.front(), Symbol{0});
    EXPECT_EQ(all.back(), Symbol{255});
}

TEST(AlphabetIterator, VisitsExactlySizeSymbols) {
    EXPECT_EQ(collected(dna).size(), dna.size());
    EXPECT_EQ(collected(digits).size(), digits.size());
    EXPECT_EQ(collected(everything).size(), everything.size());
    EXPECT_EQ(collected(Alphabet{}).size(), Alphabet{}.size());
}

TEST(AlphabetIterator, AgreesWithIndexOf) {
    std::size_t position = 0;
    for (const Symbol symbol : dna) {
        EXPECT_EQ(dna.index_of(symbol), position);
        ++position;
    }
    EXPECT_EQ(position, dna.size());
}

TEST(AlphabetIterator, StartsAtTheFirstSymbolNotAtZero) {
    const Alphabet alphabet = made({Symbol{200}});
    EXPECT_EQ(*alphabet.begin(), Symbol{200});
}

TEST(AlphabetIterator, PostfixIncrementReturnsTheOldPosition) {
    const Alphabet alphabet = Alphabet::from_symbols("ab");
    auto it = alphabet.begin();
    const auto before = it++;

    EXPECT_EQ(*before, to_symbol('a'));
    EXPECT_EQ(*it, to_symbol('b'));
}

TEST(AlphabetIterator, SupportsSeveralPasses) {
    const Alphabet alphabet = Alphabet::from_symbols("ab");
    auto first = alphabet.begin();
    auto second = first;
    ++second;

    EXPECT_EQ(*first, to_symbol('a'));
    EXPECT_EQ(*second, to_symbol('b'));
    EXPECT_EQ(collected(alphabet), collected(alphabet));
}

TEST(AlphabetIterator, WorksWithRangeAlgorithms) {
    EXPECT_EQ(std::ranges::distance(dna), 4);
    EXPECT_TRUE(std::ranges::contains(dna, to_symbol('G')));
    EXPECT_FALSE(std::ranges::contains(dna, to_symbol('B')));
}

} // namespace
} // namespace formal
