#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

#include "regex.hpp"
#include "regex/error.hpp"
#include "regex/position.hpp"

int main(int argc, char** argv) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));
    const bool dot = args.size() == 3 && std::string_view(args[1]) == "--dot";

    if (args.size() != 2 && !dot) {
        std::cerr << "usage: " << args[0] << " [--dot] <regex>\n";
        return 2;
    }

    try {
        const formal::Regex pattern(dot ? args[2] : args[1]);
        if (dot) {
            std::cout << pattern.to_dot();
        } else {
            std::cout << pattern << '\n';
        }
    } catch (const formal::regex::SyntaxError& error) {
        const formal::regex::Position position = error.position();
        std::cerr << "error at " << position.line << ':' << position.column
                  << ": " << error.what() << '\n';
        return 1;
    }

    return 0;
}
