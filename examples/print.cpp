#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "automata/complete.hpp"
#include "automata/determinize.hpp"
#include "automata/dfa.hpp"
#include "automata/dot.hpp"
#include "automata/nfa.hpp"
#include "regex.hpp"
#include "regex/dot.hpp"
#include "regex/error.hpp"
#include "regex/node.hpp"
#include "regex/parser.hpp"
#include "regex/position.hpp"
#include "regex/thompson.hpp"
#include "util/alphabet.hpp"

using namespace formal;

namespace {

void usage(std::string_view program) {
    std::cerr << "usage: " << program
              << " [--ast|--nfa|--dfa|--complete] [--alphabet SYMBOLS]"
                 " <regex>\n";
}

struct Options {
    std::string_view mode;
    Alphabet alphabet = Alphabet::all();
    std::string_view pattern;
};

std::optional<Options> parse_options(std::span<char*> args) {
    Options options;
    std::optional<std::string_view> pattern;

    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--ast" || arg == "--nfa" || arg == "--dfa" ||
            arg == "--complete") {
            options.mode = arg;
        } else if (arg == "--alphabet") {
            if (++i == args.size()) { return std::nullopt; }
            options.alphabet = Alphabet::from_symbols(args[i]);
        } else if (arg.starts_with("--") || pattern.has_value()) {
            return std::nullopt;
        } else {
            pattern = arg;
        }
    }

    if (!pattern.has_value()) { return std::nullopt; }
    options.pattern = *pattern;
    return options;
}

} // namespace

int main(int argc, char** argv) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));

    const std::optional<Options> options = parse_options(args);
    if (!options.has_value()) {
        usage(args[0]);
        return 2;
    }

    const std::string pattern(options->pattern);

    try {
        if (options->mode.empty()) {
            std::cout << Regex(pattern, options->alphabet) << '\n';
        } else {
            const regex::NodePtr tree =
                regex::parse(pattern, options->alphabet);
            if (options->mode == "--ast") {
                std::cout << to_dot(*tree);
            } else {
                const automata::Nfa nfa = to_nfa(*tree);
                if (options->mode == "--nfa") {
                    std::cout << to_dot(nfa);
                } else {
                    const automata::Dfa dfa = determinize(nfa);
                    if (options->mode == "--dfa") {
                        std::cout << to_dot(dfa);
                    } else {
                        std::cout << to_dot(complete(dfa));
                    }
                }
            }
        }
    } catch (const regex::SyntaxError& error) {
        const regex::Position position = error.position();
        std::cerr << "error at " << position.line << ':' << position.column
                  << ": " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
