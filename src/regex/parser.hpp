#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "regex/error.hpp"
#include "regex/lexer.hpp"
#include "regex/node.hpp"
#include "regex/position.hpp"
#include "regex/token.hpp"
#include "util/alphabet.hpp"

namespace formal::regex {

class Parser {
  public:
    constexpr explicit Parser(std::string input,
                              Alphabet allowed = Alphabet::all()) noexcept
        : lexer_(std::move(input), allowed) {}

    [[nodiscard]] constexpr NodePtr parse() {
        NodePtr tree = parse_union();
        if (!check(TokenType::End)) {
            const Token token = lexer_.peek();
            throw SyntaxError(unexpected(token.type()), token.position());
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

    [[nodiscard]] static std::string_view unexpected(TokenType type) {
        switch (type) {
            case TokenType::Star:
                return "Nothing to repeat before '*'";
            case TokenType::RParen:
                return "Unmatched ')'";
            case TokenType::Pipe:
            case TokenType::LParen:
            case TokenType::Letter:
            case TokenType::End:
                break;
        }
        return "Unexpected token";
    }

    [[nodiscard]] constexpr bool can_begin_atom() const {
        return check(TokenType::Letter, TokenType::LParen);
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
        if (check(TokenType::LParen)) {
            const Token open = lexer_.next();
            NodePtr inner = parse_union();
            expect(TokenType::RParen, "Missing ')'", open.position());
            return inner;
        }
        throw SyntaxError("Expected an expression", lexer_.peek().position());
    }

    Lexer lexer_;
};

[[nodiscard]] constexpr NodePtr parse(std::string pattern,
                                      Alphabet allowed = Alphabet::all()) {
    return Parser(std::move(pattern), allowed).parse();
}

} // namespace formal::regex
