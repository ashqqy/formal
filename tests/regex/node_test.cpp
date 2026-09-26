#include <string>

#include <gtest/gtest.h>

#include "regex/node.hpp"
#include "regex/visitor.hpp"
#include "util/symbol.hpp"

namespace formal::regex {
namespace {

class Counter final : public Visitor {
  public:
    constexpr void visit(const SymbolNode&) override { ++symbols; }
    constexpr void visit(const EpsilonNode&) override { ++epsilons; }

    constexpr void visit(const ConcatNode& node) override {
        ++concats;
        node.left().accept(*this);
        node.right().accept(*this);
    }

    constexpr void visit(const UnionNode& node) override {
        ++unions;
        node.left().accept(*this);
        node.right().accept(*this);
    }

    constexpr void visit(const StarNode& node) override {
        ++stars;
        node.child().accept(*this);
    }

    int symbols = 0;
    int epsilons = 0;
    int concats = 0;
    int unions = 0;
    int stars = 0;
};

class SymbolCollector final : public Visitor {
  public:
    constexpr void visit(const SymbolNode& node) override {
        out += static_cast<char>(node.symbol());
    }
    constexpr void visit(const EpsilonNode&) override { out += '@'; }

    constexpr void visit(const ConcatNode& node) override {
        node.left().accept(*this);
        node.right().accept(*this);
    }

    constexpr void visit(const UnionNode& node) override {
        node.left().accept(*this);
        node.right().accept(*this);
    }

    constexpr void visit(const StarNode& node) override {
        node.child().accept(*this);
    }

    std::string out;
};

constexpr std::string collect(const Node& root) {
    SymbolCollector collector;
    root.accept(collector);
    return collector.out;
}

TEST(Node, SymbolKeepsItsSymbol) {
    const SymbolNode node{'a'};
    EXPECT_EQ(node.symbol(), Symbol{'a'});
}

TEST(Node, VisitorReachesEveryKind) {
    const NodePtr tree = make_star(make_union(
        make_concat(make_symbol('a'), make_symbol('b')), make_epsilon()));

    Counter counter;
    tree->accept(counter);

    EXPECT_EQ(counter.symbols, 2);
    EXPECT_EQ(counter.epsilons, 1);
    EXPECT_EQ(counter.concats, 1);
    EXPECT_EQ(counter.unions, 1);
    EXPECT_EQ(counter.stars, 1);
}

TEST(Node, ConcatKeepsChildrenInOrder) {
    const NodePtr tree = make_concat(make_symbol('a'), make_symbol('b'));
    EXPECT_EQ(collect(*tree), "ab");
}

TEST(Node, UnionKeepsBranchesInOrder) {
    const NodePtr tree = make_union(make_symbol('x'), make_symbol('y'));
    EXPECT_EQ(collect(*tree), "xy");
}

TEST(Node, StarKeepsItsChild) {
    const NodePtr tree = make_star(make_symbol('z'));
    EXPECT_EQ(collect(*tree), "z");
}

TEST(Node, EpsilonHasNothingToVisit) {
    const NodePtr tree = make_epsilon();
    EXPECT_EQ(collect(*tree), "@");
}

constexpr bool visits_a_tree_at_compile_time() {
    const NodePtr tree =
        make_star(make_concat(make_symbol('a'), make_symbol('b')));
    Counter counter;
    tree->accept(counter);
    return counter.symbols == 2 && counter.concats == 1 && counter.stars == 1;
}

static_assert(visits_a_tree_at_compile_time());

TEST(Node, VisitsATreeAtRunTimeToo) {
    EXPECT_TRUE(visits_a_tree_at_compile_time());
}

} // namespace
} // namespace formal::regex
