#pragma once

#include <cstddef>
#include <utility>

#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "regex/node.hpp"
#include "regex/visitor.hpp"

namespace formal::regex {

class ThompsonBuilder final : public Visitor {
  public:
    [[nodiscard]] constexpr automata::Nfa take() && {
        nfa_.add_start(current_.start);
        nfa_.set_accepting(current_.accept);
        return std::move(nfa_);
    }

    constexpr void visit(const SymbolNode& node) override {
        const auto start = nfa_.add_state();
        const auto accept = nfa_.add_state();
        nfa_.add_transition(start, node.symbol(), accept);
        current_ = {.start = start, .accept = accept};
    }
    constexpr void visit(const EpsilonNode& /* unused */) override {
        const auto start = nfa_.add_state();
        const auto accept = nfa_.add_state();
        nfa_.add_epsilon(start, accept);
        current_ = {.start = start, .accept = accept};
    }
    constexpr void visit(const ConcatNode& node) override {
        const Fragment first = build(node.child(0));
        Fragment last = first;
        for (std::size_t i = 1; i < node.size(); ++i) {
            const Fragment next = build(node.child(i));
            nfa_.add_epsilon(last.accept, next.start);
            last = next;
        }
        current_ = {.start = first.start, .accept = last.accept};
    }
    constexpr void visit(const UnionNode& node) override {
        const auto start = nfa_.add_state();
        const auto accept = nfa_.add_state();
        for (std::size_t i = 0; i < node.size(); ++i) {
            const Fragment branch = build(node.child(i));
            nfa_.add_epsilon(start, branch.start);
            nfa_.add_epsilon(branch.accept, accept);
        }
        current_ = {.start = start, .accept = accept};
    }
    constexpr void visit(const StarNode& node) override {
        const auto start = nfa_.add_state();
        const auto accept = nfa_.add_state();
        const Fragment body = build(node.child());
        nfa_.add_epsilon(start, body.start);
        nfa_.add_epsilon(body.accept, accept);
        nfa_.add_epsilon(start, accept);
        nfa_.add_epsilon(body.accept, body.start);
        current_ = {.start = start, .accept = accept};
    }

  private:
    struct Fragment {
        automata::StateId start;
        automata::StateId accept;
    };

    [[nodiscard]] constexpr Fragment build(const Node& node) {
        node.accept(*this);
        return current_;
    }

    automata::Nfa nfa_;
    Fragment current_{};
};

[[nodiscard]] constexpr automata::Nfa to_nfa(const Node& root) {
    ThompsonBuilder builder;
    root.accept(builder);
    return std::move(builder).take();
}

} // namespace formal::regex
