#pragma once

#include <cstddef>
#include <format>
#include <string>
#include <utility>

#include "regex/node.hpp"
#include "regex/visitor.hpp"
#include "util/dot.hpp"

namespace formal::regex {

class DotPrinter final : public Visitor {
  public:
    [[nodiscard]] constexpr std::string take() && {
        return "digraph regex {\n  ordering=out;\n" + std::move(body_) + "}\n";
    }

    constexpr void visit(const SymbolNode& node) override {
        declare(dot_label(node.symbol()));
    }

    constexpr void visit(const EpsilonNode& /*unused*/) override {
        declare("Epsilon");
    }

    constexpr void visit(const ConcatNode& node) override {
        const std::size_t id = declare("Concat");
        for (std::size_t i = 0; i < node.size(); ++i) {
            draw_edge(id, node.child(i));
        }
    }

    constexpr void visit(const UnionNode& node) override {
        const std::size_t id = declare("Union");
        for (std::size_t i = 0; i < node.size(); ++i) {
            draw_edge(id, node.child(i));
        }
    }

    constexpr void visit(const StarNode& node) override {
        const std::size_t id = declare("Star");
        draw_edge(id, node.child());
    }

  private:
    constexpr std::size_t declare(const std::string& label) {
        const std::size_t id = next_id_++;
        body_ += std::format("  n{} [label=\"{}\"];\n", id, label);
        current_ = id;
        return id;
    }

    constexpr void draw_edge(std::size_t parent, const Node& child) {
        child.accept(*this);
        body_ += std::format("  n{} -> n{};\n", parent, current_);
        current_ = parent;
    }

    std::string body_;
    std::size_t next_id_ = 0;
    std::size_t current_ = 0;
};

[[nodiscard]] constexpr std::string to_dot(const Node& root) {
    DotPrinter printer;
    root.accept(printer);
    return std::move(printer).take();
}

} // namespace formal::regex
