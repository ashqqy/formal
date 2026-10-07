#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "util/symbol.hpp"

namespace formal {

class Alphabet {
  public:
    class Iterator {
      public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::input_iterator_tag;
        using value_type = Symbol;
        using difference_type = std::ptrdiff_t;
        using reference = Symbol;
        using pointer = void;

        Iterator() = default;
        constexpr Iterator(const Alphabet* alphabet, std::size_t index) noexcept
            : alphabet_(alphabet), index_(index) {
            skip_unset();
        }

        [[nodiscard]] constexpr Symbol operator*() const noexcept {
            return static_cast<Symbol>(index_);
        }
        constexpr Iterator& operator++() noexcept {
            ++index_;
            skip_unset();
            return *this;
        }
        constexpr Iterator operator++(int) noexcept {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }
        bool operator==(const Iterator&) const = default;

      private:
        constexpr void skip_unset() noexcept {
            while (index_ < kSymbolCount &&
                   !alphabet_->contains(static_cast<Symbol>(index_))) {
                ++index_;
            }
        }

        const Alphabet* alphabet_ = nullptr;
        std::size_t index_ = kSymbolCount;
    };

    [[nodiscard]] static constexpr Alphabet
    from_symbols(std::string_view symbols) noexcept {
        Alphabet alphabet;
        for (const char c : symbols) {
            alphabet.add(to_symbol(c));
        }
        return alphabet;
    }
    [[nodiscard]] static constexpr Alphabet from_range(Symbol first,
                                                       Symbol last) {
        if (first > last) {
            throw std::invalid_argument("Alphabet::from_range: first > last");
        }
        Alphabet alphabet;
        for (std::size_t s = first; s <= last; ++s) {
            alphabet.add(static_cast<Symbol>(s));
        }
        return alphabet;
    }
    [[nodiscard]] static constexpr Alphabet all() noexcept {
        static_assert(kSymbolCount % kChunkBits == 0);
        Alphabet alphabet;
        alphabet.chunks_.fill(~Chunk{0});
        return alphabet;
    }

    [[nodiscard]] constexpr Iterator begin() const noexcept {
        return {this, 0};
    }
    [[nodiscard]] constexpr Iterator end() const noexcept {
        return {this, kSymbolCount};
    }

    constexpr void add(Symbol symbol) noexcept {
        chunks_[chunk_of(symbol)] |= mask_of(symbol);
    }
    constexpr void merge(const Alphabet& other) noexcept {
        for (std::size_t i = 0; i < kChunkCount; ++i) {
            chunks_[i] |= other.chunks_[i];
        }
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
    [[nodiscard]] constexpr std::size_t index_of(Symbol symbol) const noexcept {
        assert(contains(symbol));

        std::size_t index = 0;
        for (std::size_t i = 0; i < chunk_of(symbol); ++i) {
            index += static_cast<std::size_t>(std::popcount(chunks_[i]));
        }
        index += static_cast<std::size_t>(
            std::popcount(chunks_[chunk_of(symbol)] & (mask_of(symbol) - 1)));

        return index;
    }

    bool operator==(const Alphabet&) const = default;

  private:
    using Chunk = std::uint64_t;
    static constexpr std::size_t kChunkBits =
        std::numeric_limits<Chunk>::digits;
    static constexpr std::size_t kSymbolCount =
        std::numeric_limits<Symbol>::max() + 1;
    static constexpr std::size_t kChunkCount =
        (kSymbolCount + kChunkBits - 1) / kChunkBits;

    [[nodiscard]] static constexpr std::size_t
    chunk_of(Symbol symbol) noexcept {
        return symbol / kChunkBits;
    }
    [[nodiscard]] static constexpr Chunk mask_of(Symbol symbol) noexcept {
        return Chunk{1} << (symbol % kChunkBits);
    }

    std::array<Chunk, kChunkCount> chunks_{};
};

} // namespace formal
