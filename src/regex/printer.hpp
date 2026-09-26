#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "regex/node.hpp"
#include "regex/visitor.hpp"
#include "util/symbol.hpp"

namespace formal::regex {

class Printer final : public Visitor {
  public:
    [[nodiscard]] constexpr std::string take() && { return std::move(out_); }

    constexpr void visit(const SymbolNode& node) override {
        append(node.symbol());
    }
    constexpr void visit(const EpsilonNode&) override {
        if (is_operand_position()) { out_ += "()"; }
    }
    constexpr void visit(const ConcatNode& node) override {
        const bool needs_parens = Precedence::Concat < parent_precedence_;
        if (needs_parens) { out_ += '('; }
        print_child(node.left(), Precedence::Concat);
        print_child(node.right(), Precedence::Concat);
        if (needs_parens) { out_ += ')'; }
    }
    constexpr void visit(const UnionNode& node) override {
        const bool needs_parens = Precedence::Union < parent_precedence_;
        if (needs_parens) { out_ += '('; }
        print_child(node.left(), Precedence::Union);
        out_ += '|';
        print_child(node.right(), Precedence::Union);
        if (needs_parens) { out_ += ')'; }
    }
    constexpr void visit(const StarNode& node) override {
        print_child(node.child(), Precedence::Star);
        out_ += '*';
    }

  private:
    enum class Precedence : std::uint8_t {
        Union = 1,
        Concat = 2,
        Star = 3,
    };

    constexpr void print_child(const Node& child,
                               Precedence current_precedence) {
        const Precedence parent_precedence_saved =
            std::exchange(parent_precedence_, current_precedence);
        child.accept(*this);
        parent_precedence_ = parent_precedence_saved;
    }

    constexpr void append(Symbol symbol) {
        if (symbol == '|' || symbol == '*' || symbol == '(' || symbol == ')' ||
            symbol == '\\' || symbol == ' ') {
            out_ += '\\';
        }
        out_ += static_cast<char>(symbol);
    }

    [[nodiscard]] constexpr bool is_operand_position() const {
        return parent_precedence_ == Precedence::Star;
    }

    std::string out_;
    Precedence parent_precedence_ = Precedence::Union;
};

[[nodiscard]] constexpr std::string to_string(const Node& root) {
    Printer printer;
    root.accept(printer);
    return std::move(printer).take();
}

inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    return os << to_string(node);
}

} // namespace formal::regex
