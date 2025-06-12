#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "repl.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        interp::repl::start(std::cin, std::cout);
    } else if (argc == 2) {
        if (!std::filesystem::exists(argv[1])) {
            std::println("file {} does not exist", argv[1]);
            return 1;
        }

        std::ifstream ifs{argv[1]};
        std::ostringstream oss{};
        oss << ifs.rdbuf();
        auto str = oss.str();

        if (str.empty()) {
            std::println("file {} is empty", argv[1]);
            return 1;
        }

        interp::lexer::lexer l{str};
        interp::parser::parser p{l};
        auto program{p.parse_program()};
        if (!p.errors.empty()) {
            std::println(stderr, "parser had {} errors", p.errors.size());
            for (const auto& err : p.errors) {
                std::println(stderr, "parser error: {}", err);
            }

            return 1;
        }

        interp::object::environment env{};
        auto evaluated{interp::eval::eval(program, env)};
        if (evaluated->type() == interp::object::object_type::Error) {
            std::println("{}", evaluated->to_string());
        }
    } else {
        std::println("Invalid command");
    }

    return 0;
}
