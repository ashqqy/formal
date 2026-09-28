#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "util/symbol.hpp"

namespace formal {

class Alphabet {
  public:
    constexpr void add(Symbol symbol) noexcept {
        chunks_[chunk_of(symbol)] |= mask_of(symbol);
    }
    [[nodiscard]] constexpr bool contains(Symbol symbol) const noexcept {
        return (chunks_[chunk_of(symbol)] & mask_of(symbol)) != 0;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        std::size_t count = 0;
        for (const Chunk chunk : chunks_) {
            count += static_cast<std::size_t>(std::popcount(chunk));
        }
        return count;
    }

    bool operator==(const Alphabet&) const = default;

  private:
    using Chunk = std::uint64_t;
    static constexpr std::size_t chunk_bits =
        std::numeric_limits<Chunk>::digits;
    static constexpr std::size_t symbol_count =
        std::numeric_limits<Symbol>::max() + 1;
    static constexpr std::size_t chunk_count =
        (symbol_count + chunk_bits - 1) / chunk_bits;

    [[nodiscard]] static constexpr std::size_t
    chunk_of(Symbol symbol) noexcept {
        return symbol / chunk_bits;
    }

    [[nodiscard]] static constexpr Chunk mask_of(Symbol symbol) noexcept {
        return Chunk{1} << (symbol % chunk_bits);
    }

    std::array<Chunk, chunk_count> chunks_{};
};

} // namespace formal
