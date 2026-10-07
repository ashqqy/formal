#include <gtest/gtest.h>

#include "automata/dfa.hpp"
#include "automata/dot.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/alphabet.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr std::string_view header = "digraph nfa {\n  rankdir=LR;\n";
constexpr std::string_view dfa_header = "digraph dfa {\n  rankdir=LR;\n";

Nfa two_states() {
    Nfa nfa;
    (void)nfa.add_state();
    (void)nfa.add_state();
    return nfa;
}

TEST(AutomataDot, DrawsAnEmptyAutomaton) {
    EXPECT_EQ(to_dot(Nfa{}), std::string(header) + "}\n");
}

TEST(AutomataDot, DeclaresEveryState) {
    EXPECT_EQ(to_dot(two_states()), std::string(header) +
                                        "  n0 [label=\"0\"];\n"
                                        "  n1 [label=\"1\"];\n"
                                        "}\n");
}

TEST(AutomataDot, GivesAcceptingStatesADoubleCircle) {
    Nfa nfa = two_states();
    nfa.set_accepting(1);

    EXPECT_EQ(to_dot(nfa), std::string(header) +
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\", shape=doublecircle];\n"
                               "}\n");
}

TEST(AutomataDot, PointsAnArrowAtEveryStartState) {
    Nfa nfa = two_states();
    nfa.add_start(0);
    nfa.add_start(1);

    EXPECT_EQ(to_dot(nfa), std::string(header) +
                               "  start0 [shape=point, width=0];\n"
                               "  start0 -> n0;\n"
                               "  start1 [shape=point, width=0];\n"
                               "  start1 -> n1;\n"
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "}\n");
}

TEST(AutomataDot, DrawsALabelledTransition) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_EQ(to_dot(nfa), std::string(header) + "  n0 [label=\"0\"];\n"
                                                 "  n1 [label=\"1\"];\n"
                                                 "  n0 -> n1 [label=\"a\"];\n"
                                                 "}\n");
}

TEST(AutomataDot, DrawsAnEpsilonTransition) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 1);

    EXPECT_EQ(to_dot(nfa), std::string(header) + "  n0 [label=\"0\"];\n"
                                                 "  n1 [label=\"1\"];\n"
                                                 "  n0 -> n1 [label=\"eps\"];\n"
                                                 "}\n");
}

TEST(AutomataDot, DrawsLabelledTransitionsBeforeEpsilons) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 1);
    nfa.add_transition(0, to_symbol('a'), 1);

    EXPECT_EQ(to_dot(nfa), std::string(header) + "  n0 [label=\"0\"];\n"
                                                 "  n1 [label=\"1\"];\n"
                                                 "  n0 -> n1 [label=\"a\"];\n"
                                                 "  n0 -> n1 [label=\"eps\"];\n"
                                                 "}\n");
}

TEST(AutomataDot, EscapesAQuoteInALabel) {
    Nfa nfa = two_states();
    nfa.add_transition(0, to_symbol('"'), 1);

    EXPECT_EQ(to_dot(nfa), std::string(header) +
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "  n0 -> n1 [label=\"\\\"\"];\n"
                               "}\n");
}

TEST(AutomataDot, EscapesANonPrintableSymbol) {
    Nfa nfa = two_states();
    nfa.add_transition(0, Symbol{1}, 1);

    EXPECT_EQ(to_dot(nfa), std::string(header) +
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "  n0 -> n1 [label=\"\\\\x01\"];\n"
                               "}\n");
}

TEST(AutomataDot, DrawsASelfLoop) {
    Nfa nfa = two_states();
    nfa.add_epsilon(0, 0);

    EXPECT_EQ(to_dot(nfa), std::string(header) + "  n0 [label=\"0\"];\n"
                                                 "  n1 [label=\"1\"];\n"
                                                 "  n0 -> n0 [label=\"eps\"];\n"
                                                 "}\n");
}

Dfa three_states(std::string_view symbols) {
    Dfa dfa{Alphabet::from_symbols(symbols)};
    (void)dfa.add_state();
    (void)dfa.add_state();
    (void)dfa.add_state();
    return dfa;
}

TEST(DfaDot, DrawsAnEmptyAutomaton) {
    EXPECT_EQ(to_dot(Dfa{Alphabet::from_symbols("a")}),
              std::string(dfa_header) + "}\n");
}

TEST(DfaDot, UsesItsOwnGraphName) {
    EXPECT_NE(to_dot(Dfa{Alphabet::from_symbols("a")}), to_dot(Nfa{}));
}

TEST(DfaDot, DeclaresEveryState) {
    EXPECT_EQ(to_dot(three_states("a")), std::string(dfa_header) +
                                             "  n0 [label=\"0\"];\n"
                                             "  n1 [label=\"1\"];\n"
                                             "  n2 [label=\"2\"];\n"
                                             "}\n");
}

TEST(DfaDot, DrawsNoArrowWithoutAStart) {
    const std::string drawn = to_dot(three_states("a"));
    EXPECT_EQ(drawn.find("start"), std::string::npos);
}

TEST(DfaDot, PointsAtTheStart) {
    Dfa dfa = three_states("a");
    dfa.set_start(1);

    EXPECT_EQ(to_dot(dfa), std::string(dfa_header) +
                               "  start1 [shape=point, width=0];\n"
                               "  start1 -> n1;\n"
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "  n2 [label=\"2\"];\n"
                               "}\n");
}

TEST(DfaDot, DoublesTheCircleOfAcceptingStates) {
    Dfa dfa = three_states("a");
    dfa.set_accepting(2);

    EXPECT_EQ(to_dot(dfa), std::string(dfa_header) +
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "  n2 [label=\"2\", shape=doublecircle];\n"
                               "}\n");
}

TEST(DfaDot, DrawsNothingForAHole) {
    Dfa dfa = three_states("ab");
    dfa.set_transition(0, to_symbol('a'), 1);

    const std::string drawn = to_dot(dfa);
    EXPECT_NE(drawn.find("n0 -> n1 [label=\"a\"];"), std::string::npos);
    EXPECT_EQ(drawn.find("n0 -> n2"), std::string::npos);
    EXPECT_EQ(drawn.find("n1 ->"), std::string::npos);
}

TEST(DfaDot, MergesSymbolsLeadingToTheSameTarget) {
    Dfa dfa = three_states("abc");
    dfa.set_transition(0, to_symbol('a'), 1);
    dfa.set_transition(0, to_symbol('b'), 2);
    dfa.set_transition(0, to_symbol('c'), 2);

    EXPECT_EQ(to_dot(dfa), std::string(dfa_header) +
                               "  n0 [label=\"0\"];\n"
                               "  n1 [label=\"1\"];\n"
                               "  n2 [label=\"2\"];\n"
                               "  n0 -> n1 [label=\"a\"];\n"
                               "  n0 -> n2 [label=\"b,c\"];\n"
                               "}\n");
}

TEST(DfaDot, OrdersSymbolsWithinAMergedLabel) {
    Dfa dfa = three_states("abc");
    dfa.set_transition(0, to_symbol('c'), 1);
    dfa.set_transition(0, to_symbol('a'), 1);
    dfa.set_transition(0, to_symbol('b'), 1);

    const std::string drawn = to_dot(dfa);
    EXPECT_NE(drawn.find("[label=\"a,b,c\"]"), std::string::npos);
}

TEST(DfaDot, DrawsASelfLoop) {
    Dfa dfa = three_states("a");
    dfa.set_transition(1, to_symbol('a'), 1);

    const std::string drawn = to_dot(dfa);
    EXPECT_NE(drawn.find("n1 -> n1 [label=\"a\"];"), std::string::npos);
}

TEST(DfaDot, EscapesSymbolsInLabels) {
    Dfa dfa = three_states("\"\\\x01");
    dfa.set_transition(0, to_symbol('"'), 1);
    dfa.set_transition(0, to_symbol('\\'), 1);
    dfa.set_transition(0, Symbol{1}, 1);

    const std::string drawn = to_dot(dfa);
    EXPECT_NE(drawn.find(R"([label="\\x01,\",\\"])"), std::string::npos)
        << drawn;
}

} // namespace
} // namespace formal::automata
