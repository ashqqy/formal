#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "regex/visitor.hpp"
#include "util/symbol.hpp"

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
    constexpr explicit ConcatNode(std::vector<NodePtr> children) noexcept
        : children_(std::move(children)) {
        assert(children_.size() >= 2 && "concat needs at least two children");
        for (const NodePtr& child : children_) {
            assert(child != nullptr && "concat child must not be null");
        }
    }

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& child(std::size_t idx) const noexcept {
        assert(idx < children_.size() && "child index out of range");
        return *children_[idx];
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return children_.size();
    }

  private:
    std::vector<NodePtr> children_;
};

class UnionNode final : public Node {
  public:
    constexpr explicit UnionNode(std::vector<NodePtr> children) noexcept
        : children_(std::move(children)) {
        assert(children_.size() >= 2 && "union needs at least two branches");
        for (const NodePtr& child : children_) {
            assert(child != nullptr && "union branch must not be null");
        }
    }

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& child(std::size_t idx) const noexcept {
        assert(idx < children_.size() && "child index out of range");
        return *children_[idx];
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return children_.size();
    }

  private:
    std::vector<NodePtr> children_;
};

class StarNode final : public Node {
  public:
    explicit constexpr StarNode(NodePtr child) : child_(std::move(child)) {
        assert(child_ != nullptr && "star needs a child");
    }

    constexpr void accept(Visitor& v) const override { v.visit(*this); }

    [[nodiscard]] constexpr const Node& child() const noexcept {
        return *child_;
    }

  private:
    NodePtr child_;
};


namespace detail {

template <std::same_as<NodePtr>... Children>
[[nodiscard]] constexpr std::vector<NodePtr> pack(Children... children) {
    std::vector<NodePtr> nodes;
    nodes.reserve(sizeof...(children));
    (nodes.push_back(std::move(children)), ...);
    return nodes;
}

} // namespace detail

[[nodiscard]] constexpr NodePtr make_symbol(Symbol symbol) {
    return std::make_unique<SymbolNode>(symbol);
}

[[nodiscard]] constexpr NodePtr make_epsilon() {
    return std::make_unique<EpsilonNode>();
}

[[nodiscard]] constexpr NodePtr make_star(NodePtr child) {
    return std::make_unique<StarNode>(std::move(child));
}

[[nodiscard]] constexpr NodePtr make_concat(std::vector<NodePtr> children) {
    if (children.empty()) { return make_epsilon(); }
    if (children.size() == 1) {
        assert(children.front() != nullptr && "a child must not be null");
        return std::move(children.front());
    }
    return std::make_unique<ConcatNode>(std::move(children));
}

[[nodiscard]] constexpr NodePtr make_union(std::vector<NodePtr> children) {
    assert(!children.empty() && "an empty union would be the empty language");
    if (children.size() == 1) {
        assert(children.front() != nullptr && "a child must not be null");
        return std::move(children.front());
    }
    return std::make_unique<UnionNode>(std::move(children));
}

template <std::same_as<NodePtr>... Children>
[[nodiscard]] constexpr NodePtr make_concat(Children... children) {
    return make_concat(detail::pack(std::move(children)...));
}

template <std::same_as<NodePtr>... Children>
[[nodiscard]] constexpr NodePtr make_union(Children... children) {
    static_assert(sizeof...(Children) >= 1,
                  "a union needs at least one branch");
    return make_union(detail::pack(std::move(children)...));
}

} // namespace formal::regex
