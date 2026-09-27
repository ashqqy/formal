#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

#include "regex/error.hpp"
#include "regex/lexer.hpp"
#include "regex/node.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"

namespace formal::regex {

class Parser {
  public:
    constexpr explicit Parser(std::string input) noexcept
        : lexer_(std::move(input)) {}

    [[nodiscard]] constexpr NodePtr parse() {
        NodePtr tree = parse_union();
        if (!check(TokenType::End)) {
            throw SyntaxError("Unexpected token", lexer_.peek().position());
        }
        return tree;
    }

  private:
    template <std::same_as<TokenType>... Types>
    [[nodiscard]] constexpr bool check(Types... types) const {
        static_assert(sizeof...(Types) >= 1, "Expected at least one type");
        const TokenType current = lexer_.peek().type();
        return ((current == types) || ...);
    }
    constexpr bool try_consume(TokenType type) {
        if (check(type)) {
            (void)lexer_.next();
            return true;
        }
        return false;
    }

    constexpr void expect(TokenType type, const char* message,
                          Position position) {
        if (!try_consume(type)) { throw SyntaxError(message, position); }
    }

    [[nodiscard]] constexpr bool can_begin_atom() const {
        return check(TokenType::Letter, TokenType::Lparen);
    }

    [[nodiscard]] constexpr NodePtr parse_union() {
        std::vector<NodePtr> branches;
        branches.push_back(parse_concat());
        while (try_consume(TokenType::Pipe)) {
            branches.push_back(parse_concat());
        }
        return make_union(std::move(branches));
    }

    [[nodiscard]] constexpr NodePtr parse_concat() {
        std::vector<NodePtr> children;
        while (can_begin_atom()) {
            children.push_back(parse_star());
        }
        return make_concat(std::move(children));
    }

    [[nodiscard]] constexpr NodePtr parse_star() {
        NodePtr node = parse_atom();
        while (try_consume(TokenType::Star)) {
            node = make_star(std::move(node));
        }
        return node;
    }

    [[nodiscard]] constexpr NodePtr parse_atom() {
        if (check(TokenType::Letter)) {
            return make_symbol(lexer_.next().symbol());
        }
        if (check(TokenType::Lparen)) {
            const Token open = lexer_.next();
            NodePtr inner = parse_union();
            expect(TokenType::Rparen, "Missing ')'", open.position());
            return inner;
        }
        throw SyntaxError("Expected an expression", lexer_.peek().position());
    }

    Lexer lexer_;
};

[[nodiscard]] constexpr NodePtr parse(std::string pattern) {
    return Parser(std::move(pattern)).parse();
}

} // namespace formal::regex
