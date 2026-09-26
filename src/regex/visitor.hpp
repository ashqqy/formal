#pragma once

namespace formal::regex {

class SymbolNode;
class EpsilonNode;
class ConcatNode;
class UnionNode;
class StarNode;

class Visitor {
  protected:
    ~Visitor() = default;

  public:
    constexpr virtual void visit(const SymbolNode&) = 0;
    constexpr virtual void visit(const EpsilonNode&) = 0;
    constexpr virtual void visit(const ConcatNode&) = 0;
    constexpr virtual void visit(const UnionNode&) = 0;
    constexpr virtual void visit(const StarNode&) = 0;
};

} // namespace formal::regex
