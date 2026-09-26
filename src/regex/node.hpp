#pragma once

#include "regex/visitor.hpp"
#include "util/symbol.hpp"

#include <memory>
#include <utility>

namespace formal::regex {

class Node {
  public:
    constexpr virtual ~Node() = default;

    constexpr virtual void accept(Visitor& visitor) const = 0;
};

using NodePtr = std::unique_ptr<Node>;


class SymbolNode final : public Node {
  public:
    explicit constexpr SymbolNode(Symbol symbol) : symbol_(symbol) {}

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr Symbol symbol() const noexcept { return symbol_; }

  private:
    Symbol symbol_;
};

class EpsilonNode final : public Node {
  public:
    constexpr void accept(Visitor& v) const override { v.visit(*this); }
};

class ConcatNode final : public Node {
  public:
    constexpr ConcatNode(NodePtr left, NodePtr right)
        : left_(std::move(left)), right_(std::move(right)) {}

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& left() const noexcept { return *left_; }
    [[nodiscard]] constexpr const Node& right() const noexcept {
        return *right_;
    }

  private:
    NodePtr left_;
    NodePtr right_;
};

class UnionNode final : public Node {
  public:
    constexpr UnionNode(NodePtr left, NodePtr right)
        : left_(std::move(left)), right_(std::move(right)) {}

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& left() const noexcept { return *left_; }
    [[nodiscard]] constexpr const Node& right() const noexcept {
        return *right_;
    }

  private:
    NodePtr left_;
    NodePtr right_;
};

class StarNode final : public Node {
  public:
    explicit constexpr StarNode(NodePtr child) : child_(std::move(child)) {}

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& child() const noexcept {
        return *child_;
    }

  private:
    NodePtr child_;
};


[[nodiscard]] constexpr NodePtr make_symbol(Symbol symbol) {
    return std::make_unique<SymbolNode>(symbol);
}

[[nodiscard]] constexpr NodePtr make_epsilon() {
    return std::make_unique<EpsilonNode>();
}

[[nodiscard]] constexpr NodePtr make_concat(NodePtr left, NodePtr right) {
    return std::make_unique<ConcatNode>(std::move(left), std::move(right));
}

[[nodiscard]] constexpr NodePtr make_union(NodePtr left, NodePtr right) {
    return std::make_unique<UnionNode>(std::move(left), std::move(right));
}

[[nodiscard]] constexpr NodePtr make_star(NodePtr child) {
    return std::make_unique<StarNode>(std::move(child));
}

} // namespace formal::regex
