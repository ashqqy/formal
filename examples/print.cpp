#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

#include "regex.hpp"
#include "regex/dot.hpp"
#include "regex/error.hpp"
#include "regex/node.hpp"
#include "regex/parser.hpp"
#include "regex/position.hpp"

int main(int argc, char** argv) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));
    const bool dot = args.size() == 3 && std::string_view(args[1]) == "--dot";

    if (args.size() != 2 && !dot) {
        std::cerr << "usage: " << args[0] << " [--dot] <regex>\n";
        return 2;
    }

    try {
        if (dot) {
            const formal::regex::NodePtr tree = formal::regex::parse(args[2]);
            std::cout << formal::regex::to_dot(*tree);
        } else {
            std::cout << formal::Regex(args[1]) << '\n';
        }
    } catch (const formal::regex::SyntaxError& error) {
        const formal::regex::Position position = error.position();
        std::cerr << "error at " << position.line << ':' << position.column
                  << ": " << error.what() << '\n';
        return 1;
    }

    return 0;
}
