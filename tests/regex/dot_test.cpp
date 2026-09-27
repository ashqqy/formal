#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "regex/dot.hpp"
#include "regex/node.hpp"
#include "regex/parser.hpp"

namespace formal::regex {
namespace {

std::string to_dot(const Node& root) {
    DotPrinter printer;
    root.accept(printer);
    return std::move(printer).take();
}

TEST(Dot, DrawsASymbol) {
    EXPECT_EQ(to_dot(*make_symbol('a')), "digraph regex {\n"
                                         "  ordering=out;\n"
                                         "  n0 [label=\"a\"];\n"
                                         "}\n");
}

TEST(Dot, DrawsEpsilon) {
    EXPECT_EQ(to_dot(*make_epsilon()), "digraph regex {\n"
                                       "  ordering=out;\n"
                                       "  n0 [label=\"Epsilon\"];\n"
                                       "}\n");
}

TEST(Dot, KeepsChildrenInOrder) {
    const NodePtr tree =
        make_concat(make_symbol('a'), make_symbol('b'), make_symbol('c'));

    EXPECT_EQ(to_dot(*tree), "digraph regex {\n"
                             "  ordering=out;\n"
                             "  n0 [label=\"Concat\"];\n"
                             "  n1 [label=\"a\"];\n"
                             "  n0 -> n1;\n"
                             "  n2 [label=\"b\"];\n"
                             "  n0 -> n2;\n"
                             "  n3 [label=\"c\"];\n"
                             "  n0 -> n3;\n"
                             "}\n");
}

TEST(Dot, DrawsAStarOverAUnion) {
    const NodePtr tree =
        make_star(make_union(make_symbol('a'), make_symbol('b')));

    EXPECT_EQ(to_dot(*tree), "digraph regex {\n"
                             "  ordering=out;\n"
                             "  n0 [label=\"Star\"];\n"
                             "  n1 [label=\"Union\"];\n"
                             "  n2 [label=\"a\"];\n"
                             "  n1 -> n2;\n"
                             "  n3 [label=\"b\"];\n"
                             "  n1 -> n3;\n"
                             "  n0 -> n1;\n"
                             "}\n");
}

TEST(Dot, EscapesQuotesAndBackslashes) {
    EXPECT_EQ(to_dot(*make_symbol('"')), "digraph regex {\n"
                                         "  ordering=out;\n"
                                         "  n0 [label=\"\\\"\"];\n"
                                         "}\n");

    EXPECT_EQ(to_dot(*make_symbol('\\')), "digraph regex {\n"
                                          "  ordering=out;\n"
                                          "  n0 [label=\"\\\\\"];\n"
                                          "}\n");
}

TEST(Dot, EscapesNonPrintableSymbols) {
    EXPECT_EQ(to_dot(*make_symbol(Symbol{1})), "digraph regex {\n"
                                               "  ordering=out;\n"
                                               "  n0 [label=\"\\\\x01\"];\n"
                                               "}\n");
}

TEST(Dot, DrawsAParsedPattern) {
    EXPECT_EQ(to_dot(*parse("ab*")), "digraph regex {\n"
                                     "  ordering=out;\n"
                                     "  n0 [label=\"Concat\"];\n"
                                     "  n1 [label=\"a\"];\n"
                                     "  n0 -> n1;\n"
                                     "  n2 [label=\"Star\"];\n"
                                     "  n3 [label=\"b\"];\n"
                                     "  n2 -> n3;\n"
                                     "  n0 -> n2;\n"
                                     "}\n");
}

} // namespace
} // namespace formal::regex
