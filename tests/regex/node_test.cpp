#include <cstddef>
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
        visit_children(node);
    }

    constexpr void visit(const UnionNode& node) override {
        ++unions;
        visit_children(node);
    }

    constexpr void visit(const StarNode& node) override {
        ++stars;
        node.child().accept(*this);
    }

  private:
    template <class N>
    constexpr void visit_children(const N& node) {
        for (std::size_t i = 0; i < node.size(); ++i) {
            node.child(i).accept(*this);
        }
    }

  public:
    int symbols = 0;
    int epsilons = 0;
    int concats = 0;
    int unions = 0;
    int stars = 0;
};

class SymbolCollector final : public Visitor {
  public:
    constexpr void visit(const SymbolNode& node) override {
        out += to_char(node.symbol());
    }
    constexpr void visit(const EpsilonNode& /*unused*/) override { out += '@'; }

    constexpr void visit(const ConcatNode& node) override {
        visit_children(node);
    }

    constexpr void visit(const UnionNode& node) override {
        visit_children(node);
    }

    constexpr void visit(const StarNode& node) override {
        node.child().accept(*this);
    }

  private:
    template <class N>
    constexpr void visit_children(const N& node) {
        for (std::size_t i = 0; i < node.size(); ++i) {
            node.child(i).accept(*this);
        }
    }

  public:
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

TEST(Node, ConcatOfThreeIsFlat) {
    const NodePtr tree =
        make_concat(make_symbol('a'), make_symbol('b'), make_symbol('c'));

    Counter counter;
    tree->accept(counter);

    EXPECT_EQ(counter.concats, 1);
    EXPECT_EQ(counter.symbols, 3);
    EXPECT_EQ(collect(*tree), "abc");
}

TEST(Node, UnionOfThreeIsFlat) {
    const NodePtr tree =
        make_union(make_symbol('a'), make_symbol('b'), make_symbol('c'));

    Counter counter;
    tree->accept(counter);

    EXPECT_EQ(counter.unions, 1);
    EXPECT_EQ(counter.symbols, 3);
    EXPECT_EQ(collect(*tree), "abc");
}

TEST(Node, ConcatOfNothingIsEpsilon) {
    Counter counter;
    make_concat()->accept(counter);
    EXPECT_EQ(counter.epsilons, 1);
    EXPECT_EQ(counter.concats, 0);
}

TEST(Node, ASingleChildIsNotWrapped) {
    Counter concat_counter;
    make_concat(make_symbol('a'))->accept(concat_counter);
    EXPECT_EQ(concat_counter.symbols, 1);
    EXPECT_EQ(concat_counter.concats, 0);

    Counter union_counter;
    make_union(make_symbol('a'))->accept(union_counter);
    EXPECT_EQ(union_counter.symbols, 1);
    EXPECT_EQ(union_counter.unions, 0);
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
