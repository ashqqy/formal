#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

#include "automata/state.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

class Dfa {
  public:
    explicit constexpr Dfa(Alphabet alphabet) : alphabet_(alphabet) {}

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return state_count_;
    }
    [[nodiscard]] constexpr StateId transition(StateId from,
                                               Symbol symbol) const noexcept {
        assert(from < state_count_);
        if (!alphabet_.contains(symbol)) { return kNoState; }
        return table_[cell_of(from, symbol)];
    }
    [[nodiscard]] constexpr Alphabet alphabet() const noexcept {
        return alphabet_;
    }

    constexpr StateId add_state() {
        assert(state_count_ < kNoState);
        table_.resize(table_.size() + alphabet_.size(), kNoState);
        return to_state_id(state_count_++);
    }
    constexpr void set_transition(StateId from, Symbol symbol,
                                  StateId to) noexcept {
        assert(from < state_count_);
        assert(to < state_count_);
        assert(alphabet_.contains(symbol));
        table_[cell_of(from, symbol)] = to;
    }

  private:
    [[nodiscard]] constexpr std::size_t cell_of(StateId state,
                                                Symbol symbol) const noexcept {
        return (state * alphabet_.size()) + alphabet_.index_of(symbol);
    }

    std::vector<StateId> table_; // size = |states| x |alphabet|
    std::size_t state_count_ = 0;
    Alphabet alphabet_;
};

} // namespace formal::automata
