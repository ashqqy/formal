#pragma once

#include <span>
#include <vector>

#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

[[nodiscard]] constexpr std::vector<StateId>
step(const Nfa& nfa, std::span<const StateId> states, Symbol symbol) {
    std::vector<bool> targets(nfa.size(), false);

    for (const auto state : states) {
        for (const auto& transition : nfa.transitions(state)) {
            if (transition.symbol == symbol) {
                targets[transition.target] = true;
            }
        }
    }

    std::vector<StateId> result;
    for (StateId id = 0; id < targets.size(); ++id) {
        if (targets[id]) { result.push_back(id); }
    }

    return result;
}

} // namespace formal::automata
