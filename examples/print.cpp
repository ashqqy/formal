#include <iostream>
#include <span>
#include <string>

#include "regex/error.hpp"
#include "regex/parser.hpp"
#include "regex/printer.hpp"

int main(int argc, char** argv) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));
    if (args.size() != 2) {
        std::cerr << "usage: " << args[0] << " <regex>\n";
        return 2;
    }

    try {
        const formal::regex::NodePtr tree = formal::regex::parse(args[1]);
        std::cout << *tree << '\n';
    } catch (const formal::regex::SyntaxError& error) {
        const formal::regex::Position position = error.position();
        std::cerr << "error at " << position.line << ':' << position.column
                  << ": " << error.what() << '\n';
        return 1;
    }

    return 0;
}
