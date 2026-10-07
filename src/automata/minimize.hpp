#pragma once

#include "automata/complete.hpp"
#include "automata/dfa.hpp"
#include "automata/state.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace formal::automata {

[[nodiscard]] constexpr Dfa minimize(const Dfa& dfa) {
    const Dfa full = complete(dfa);
    if (full.start() == kNoState) { return full; }

    const Alphabet alphabet = full.alphabet();
    std::vector<std::size_t> class_of(full.size(), 0);
    for (StateId id = 0; id < full.size(); ++id) {
        if (full.is_accepting(id)) { class_of[id] = 1; }
    }

    std::size_t class_count = 0;
    while (true) {
        std::vector<std::vector<std::size_t>> behaviour(full.size());
        for (StateId id = 0; id < full.size(); ++id) {
            behaviour[id].push_back(class_of[id]);
            for (const Symbol symbol : alphabet) {
                behaviour[id].push_back(class_of[full.transition(id, symbol)]);
            }
        }

        std::vector<std::vector<std::size_t>> class_behaviour;
        for (StateId id = 0; id < full.size(); ++id) {
            const auto found =
                std::ranges::find(class_behaviour, behaviour[id]);
            class_of[id] =
                static_cast<std::size_t>(found - class_behaviour.begin());
            if (found == class_behaviour.end()) {
                class_behaviour.push_back(behaviour[id]);
            }
        }

        if (class_behaviour.size() == class_count) { break; }
        class_count = class_behaviour.size();
    }


    Dfa result(alphabet);
    for (std::size_t i = 0; i < class_count; ++i) {
        (void)result.add_state();
    }
    for (StateId id = 0; id < full.size(); ++id) {
        const StateId from = to_state_id(class_of[id]);
        if (full.is_accepting(id)) { result.set_accepting(from); }
        for (const Symbol symbol : alphabet) {
            result.set_transition(
                from, symbol,
                to_state_id(class_of[full.transition(id, symbol)]));
        }
    }

    result.set_start(to_state_id(class_of[full.start()]));

    return result;
}

} // namespace formal::automata
