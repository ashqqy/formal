#pragma once

#include <algorithm>
#include <string_view>
#include <vector>

#include "automata/closure.hpp"
#include "automata/dfa.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "automata/step.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

[[nodiscard]] constexpr bool matches(const Nfa& nfa, std::string_view input) {
    std::vector<StateId> current = epsilon_closure(nfa, nfa.starts());

    for (const char c : input) {
        if (current.empty()) { return false; }
        current = epsilon_closure(nfa, step(nfa, current, to_symbol(c)));
    }

    return std::ranges::any_of(current, [&nfa](const StateId state) {
        return nfa.is_accepting(state);
    });
}

[[nodiscard]] constexpr bool matches(const Dfa& dfa, std::string_view input) {
    StateId current = dfa.start();
    if (current == kNoState) { return false; }

    for (const char c : input) {
        current = dfa.transition(current, to_symbol(c));
        if (current == kNoState) { return false; }
    }

    return dfa.is_accepting(current);
}

} // namespace formal::automata
