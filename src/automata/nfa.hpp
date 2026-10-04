#pragma once

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

#include "automata/state.hpp"
#include "util/sorted_set.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

class Nfa {
  public:
    struct Transition {
        Symbol symbol;
        StateId target;
        auto operator<=>(const Transition&) const = default;
    };

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return states_.size();
    }

    [[nodiscard]] constexpr std::span<const Transition>
    transitions(StateId id) const {
        return state(id).transitions.items();
    }

    [[nodiscard]] constexpr std::span<const StateId>
    epsilons(StateId id) const {
        return state(id).epsilons.items();
    }

    [[nodiscard]] constexpr StateId add_state() {
        const StateId id = to_state_id(states_.size());
        states_.emplace_back();
        return id;
    }
    constexpr void add_transition(StateId from, Symbol symbol, StateId to) {
        assert(to < size() && "Unknown target state");
        state(from).transitions.insert({.symbol = symbol, .target = to});
    }

    constexpr void add_epsilon(StateId from, StateId to) {
        assert(to < size() && "Unknown target state");
        state(from).epsilons.insert(to);
    }

  private:
    struct State {
        SortedSet<Transition> transitions;
        SortedSet<StateId> epsilons;
    };

    [[nodiscard]] constexpr State& state(StateId id) {
        assert(id < size() && "Unknown state");
        return states_[id];
    }
    [[nodiscard]] constexpr const State& state(StateId id) const {
        assert(id < size() && "Unknown state");
        return states_[id];
    }

    std::vector<State> states_;
};

} // namespace formal::automata
