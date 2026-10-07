#pragma once

#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "automata/dfa.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/dot.hpp"
#include "util/symbol.hpp"

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

[[nodiscard]] inline std::string to_dot(const Dfa& dfa) {
    std::string out = "digraph dfa {\n  rankdir=LR;\n";

    if (dfa.start() != kNoState) {
        out += std::format("  start{0} [shape=point, width=0];\n"
                           "  start{0} -> n{0};\n",
                           dfa.start());
    }
    for (StateId id = 0; id < dfa.size(); ++id) {
        out += std::format("  n{} [label=\"{}\"", id, id);
        if (dfa.is_accepting(id)) { out += ", shape=doublecircle"; }
        out += "];\n";
    }

    struct Group {
        StateId target;
        std::string label;
    };
    for (StateId id = 0; id < dfa.size(); ++id) {
        std::vector<Group> grouped;
        for (const Symbol symbol : dfa.alphabet()) {
            const StateId target = dfa.transition(id, symbol);
            if (target == kNoState) { continue; }

            const auto found =
                std::ranges::find(grouped, target, &Group::target);
            if (found == grouped.end()) {
                grouped.push_back(
                    {.target = target, .label = dot_label(symbol)});
            } else {
                found->label += ',';
                found->label += dot_label(symbol);
            }
        }

        for (const Group& group : grouped) {
            out += std::format("  n{} -> n{} [label=\"{}\"];\n", id,
                               group.target, group.label);
        }
    }

    out += "}\n";
    return out;
}

} // namespace formal::automata
