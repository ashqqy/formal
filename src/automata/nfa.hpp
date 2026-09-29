#pragma once

#include <cstddef>
#include <vector>

#include "automata/state.hpp"

namespace formal::automata {

class Nfa {
  public:
    [[nodiscard]] constexpr StateId add_state() {
        const StateId id = to_state_id(states_.size());
        states_.emplace_back();
        return id;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return states_.size();
    }

  private:
    struct State {};

    std::vector<State> states_;
};

} // namespace formal::automata
