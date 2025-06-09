#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "repl.h"

#include <fstream>
#include <iostream>
#include <print>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        interp::repl::start(std::cin, std::cout);
    } else if (argc == 2) {
        std::ifstream ifs{argv[1]};
        std::ostringstream oss{};
        oss << ifs.rdbuf();

        interp::lexer::lexer l{oss.str()};
        interp::parser::parser p{l};
        auto program{p.parse_program()};
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
