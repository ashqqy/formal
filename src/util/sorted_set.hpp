#pragma once

#include <algorithm>
#include <concepts>
#include <span>
#include <vector>

namespace formal {

template <std::totally_ordered T>
class SortedSet {
  public:
    constexpr void insert(const T& value) {
        const auto position = std::ranges::lower_bound(items_, value);
        if (position == items_.end() || *position != value) {
            items_.insert(position, value);
        }
    }

    [[nodiscard]] constexpr std::span<const T> items() const noexcept {
        return items_;
    }

    bool operator==(const SortedSet&) const = default;

  private:
    std::vector<T> items_;
};

} // namespace formal
