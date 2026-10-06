#pragma once

#include <span>
#include <vector>

#include "automata/nfa.hpp"
#include "automata/state.hpp"

namespace formal::automata {

[[nodiscard]] constexpr std::vector<StateId>
epsilon_closure(const Nfa& nfa, std::span<const StateId> states) {
    std::vector<bool> visited(nfa.size(), false);
    std::vector<StateId> stack;

    for (const StateId state : states) {
        if (!visited[state]) {
            visited[state] = true;
            stack.push_back(state);
        }
    }

    while (!stack.empty()) {
        const StateId state = stack.back();
        stack.pop_back();
        for (const StateId target : nfa.epsilons(state)) {
            if (!visited[target]) {
                visited[target] = true;
                stack.push_back(target);
            }
        }
    }

    std::vector<StateId> closure;
    for (StateId id = 0; id < nfa.size(); ++id) {
        if (visited[id]) { closure.push_back(id); }
    }

    return closure;
}

} // namespace formal::automata
