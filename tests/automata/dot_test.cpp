#include <gtest/gtest.h>

#include "automata/dot.hpp"
#include "automata/nfa.hpp"
#include "automata/state.hpp"
#include "util/symbol.hpp"

namespace formal::automata {
namespace {

constexpr std::string_view header = "digraph nfa {\n  rankdir=LR;\n";

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

} // namespace
} // namespace formal::automata
