#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace formal::automata {

using StateId = std::uint32_t;

inline constexpr StateId kNoState = std::numeric_limits<StateId>::max();

[[nodiscard]] constexpr StateId to_state_id(std::size_t index) noexcept {
    assert(index < kNoState);
    return static_cast<StateId>(index);
}

} // namespace formal::automata
