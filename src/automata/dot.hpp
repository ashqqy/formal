#pragma once

#include <format>
#include <string>

#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/dot.hpp"

namespace formal::automata {

[[nodiscard]] inline std::string to_dot(const Nfa& nfa) {
    std::string out = "digraph nfa {\n  rankdir=LR;\n";

    for (const StateId start : nfa.starts()) {
        out += std::format("  start{0} [shape=point, width=0];\n"
                           "  start{0} -> n{0};\n",
                           start);
    }

    for (StateId id = 0; id < nfa.size(); ++id) {
        out += std::format("  n{} [label=\"{}\"", id, id);
        if (nfa.is_accepting(id)) { out += ", shape=doublecircle"; }
        out += "];\n";
    }

    for (StateId id = 0; id < nfa.size(); ++id) {
        for (const Nfa::Transition& transition : nfa.transitions(id)) {
            out += std::format("  n{} -> n{} [label=\"{}\"];\n", id,
                               transition.target, dot_label(transition.symbol));
        }
        for (const StateId target : nfa.epsilons(id)) {
            out += std::format("  n{} -> n{} [label=\"eps\"];\n", id, target);
        }
    }

    out += "}\n";

    return out;
}

} // namespace formal::automata
