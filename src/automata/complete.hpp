#pragma once

#include "automata/dfa.hpp"
#include "automata/state.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

[[nodiscard]] constexpr bool is_complete(const Dfa& dfa) noexcept {
    const Alphabet alphabet = dfa.alphabet();
    for (StateId id = 0; id < dfa.size(); ++id) {
        for (const Symbol symbol : alphabet) {
            if (dfa.transition(id, symbol) == kNoState) { return false; }
        }
    }
    return true;
}

[[nodiscard]] constexpr Dfa complete(const Dfa& dfa) {
    if (is_complete(dfa)) { return dfa; }

    const Alphabet alphabet = dfa.alphabet();

    Dfa result = dfa;
    const StateId sink = result.add_state();

    for (StateId id = 0; id < result.size(); ++id) {
        for (const Symbol symbol : alphabet) {
            if (result.transition(id, symbol) == kNoState) {
                result.set_transition(id, symbol, sink);
            }
        }
    }

    return result;
}

} // namespace formal::automata
