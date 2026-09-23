#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "util/enum_name.hpp"

namespace formal {
namespace {

enum class Color : std::uint8_t { Red, Green, Blue };

static_assert(enum_name(Color::Red) == "Red");
static_assert(enum_name(Color::Green) == "Green");
static_assert(enum_name(Color::Blue) == "Blue");
static_assert(enum_name(static_cast<Color>(42)) == "<unknown>");

TEST(EnumName, NamesAnEnumerator) {
    EXPECT_EQ(enum_name(Color::Green), "Green");
}

TEST(EnumName, UnknownValueHasAPlaceholder) {
    EXPECT_EQ(enum_name(static_cast<Color>(42)), "<unknown>");
}

} // namespace
} // namespace formal
