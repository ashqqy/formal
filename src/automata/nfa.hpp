#pragma once

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

#include "automata/state.hpp"
#include "util/alphabet.hpp"
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
    transitions(StateId id) const noexcept {
        return state(id).transitions.items();
    }
    [[nodiscard]] constexpr std::span<const StateId>
    epsilons(StateId id) const noexcept {
        return state(id).epsilons.items();
    }
    [[nodiscard]] constexpr bool is_accepting(StateId id) const noexcept {
        return state(id).accepting;
    }
    [[nodiscard]] constexpr std::span<const StateId> starts() const noexcept {
        return starts_.items();
    }
    [[nodiscard]] constexpr Alphabet alphabet() const noexcept {
        return alphabet_;
    }

    [[nodiscard]] constexpr StateId add_state() {
        const StateId id = to_state_id(states_.size());
        states_.emplace_back();
        return id;
    }
    constexpr void add_transition(StateId from, Symbol symbol, StateId to) {
        assert(to < size() && "Unknown target state");
        state(from).transitions.insert({.symbol = symbol, .target = to});
        alphabet_.add(symbol);
    }
    constexpr void add_epsilon(StateId from, StateId to) {
        assert(to < size() && "Unknown target state");
        state(from).epsilons.insert(to);
    }
    constexpr void set_accepting(StateId id, bool value = true) noexcept {
        state(id).accepting = value;
    }
    constexpr void add_start(StateId id) {
        assert(id < size() && "Unknown state");
        starts_.insert(id);
    }
    constexpr void widen_alphabet(const Alphabet& other) noexcept {
        alphabet_.merge(other);
    }

    bool operator==(const Nfa&) const = default;

  private:
    struct State {
        SortedSet<Transition> transitions;
        SortedSet<StateId> epsilons;
        bool accepting = false;

        bool operator==(const State&) const = default;
    };

    [[nodiscard]] constexpr State& state(StateId id) noexcept {
        assert(id < size() && "Unknown state");
        return states_[id];
    }
    [[nodiscard]] constexpr const State& state(StateId id) const noexcept {
        assert(id < size() && "Unknown state");
        return states_[id];
    }

    std::vector<State> states_;
    SortedSet<StateId> starts_;
    Alphabet alphabet_;
};

} // namespace formal::automata
