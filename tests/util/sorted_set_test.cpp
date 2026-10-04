#include <compare>
#include <cstdint>

#include <gtest/gtest.h>

#include "util/sorted_set.hpp"

namespace formal {
namespace {

struct Pair {
    int first;
    int second;
    auto operator<=>(const Pair&) const = default;
};

template <typename T>
constexpr SortedSet<T> made(std::initializer_list<T> values) {
    SortedSet<T> set;
    for (const T& value : values) {
        set.insert(value);
    }
    return set;
}

static_assert(made<int>({}).items().empty());
static_assert(made({3, 1, 2}).items().size() == 3);
static_assert(made({1, 1, 1}).items().size() == 1);
static_assert(made({3, 1, 2}) == made({2, 3, 1}));

TEST(SortedSet, StartsEmpty) {
    const SortedSet<int> set;
    EXPECT_TRUE(set.items().empty());
}

TEST(SortedSet, SortsRegardlessOfInsertionOrder) {
    const SortedSet<int> set = made({5, 1, 3});
    ASSERT_EQ(set.items().size(), 3U);
    EXPECT_EQ(set.items()[0], 1);
    EXPECT_EQ(set.items()[1], 3);
    EXPECT_EQ(set.items()[2], 5);
}

TEST(SortedSet, IgnoresDuplicates) {
    const SortedSet<int> set = made({2, 2, 2});
    ASSERT_EQ(set.items().size(), 1U);
    EXPECT_EQ(set.items()[0], 2);
}

TEST(SortedSet, ComparesByContentNotByOrderOfInsertion) {
    EXPECT_EQ(made({1, 2, 3}), made({3, 2, 1}));
    EXPECT_NE(made({1, 2}), made({1, 2, 3}));
    EXPECT_NE(made({1, 2}), SortedSet<int>{});
}

// Поля сравниваются в порядке объявления, значит сначала по first.
TEST(SortedSet, OrdersCompoundValuesFieldByField) {
    const SortedSet<Pair> set = made<Pair>({{2, 0}, {1, 9}, {1, 4}});
    ASSERT_EQ(set.items().size(), 3U);
    EXPECT_EQ(set.items()[0], (Pair{1, 4}));
    EXPECT_EQ(set.items()[1], (Pair{1, 9}));
    EXPECT_EQ(set.items()[2], (Pair{2, 0}));
}

TEST(SortedSet, HoldsEveryValueOfASmallType) {
    SortedSet<std::uint8_t> set;
    for (int value = 255; value >= 0; --value) {
        set.insert(static_cast<std::uint8_t>(value));
    }
    ASSERT_EQ(set.items().size(), 256U);
    EXPECT_EQ(set.items()[0], 0);
    EXPECT_EQ(set.items()[255], 255);
}

} // namespace
} // namespace formal
