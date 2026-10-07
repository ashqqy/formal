#pragma once

#include <algorithm>
#include <vector>

#include "automata/closure.hpp"
#include "automata/dfa.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "automata/step.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {

[[nodiscard]] constexpr Dfa determinize(const Nfa& nfa) {
    const Alphabet alphabet = nfa.alphabet();

    struct Entry {
        std::vector<StateId> subset;
        StateId id;
    };
    Dfa dfa(alphabet);
    std::vector<std::vector<StateId>> subset_of;
    std::vector<Entry> numbering;

    const auto accepts = [&nfa](const std::vector<StateId>& subset) -> bool {
        return std::ranges::any_of(subset, [&nfa](const StateId state) {
            return nfa.is_accepting(state);
        });
    };

    const auto id_of = [&](const std::vector<StateId>& subset) -> StateId {
        const auto position =
            std::ranges::lower_bound(numbering, subset, {}, &Entry::subset);
        if (position != numbering.end() && position->subset == subset) {
            return position->id;
        }

        const StateId id = dfa.add_state();
        subset_of.push_back(subset);
        numbering.insert(position, Entry{.subset = subset, .id = id});
        if (accepts(subset)) { dfa.set_accepting(id); }
        return id;
    };

    dfa.set_start(id_of(epsilon_closure(nfa, nfa.starts())));

    for (StateId id = 0; id < dfa.size(); ++id) {
        std::vector<StateId> subset = subset_of[id];
        for (const Symbol symbol : alphabet) {
            const std::vector<StateId> next =
                epsilon_closure(nfa, step(nfa, subset, symbol));
            if (next.empty()) { continue; }
            dfa.set_transition(id, symbol, id_of(next));
        }
    }

    return dfa;
}

} // namespace formal::automata
