#include "repl.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"

#include <iostream>
#include <string_view>

namespace interp {

namespace repl {

void start(std::istream& is, std::ostream& os) {
    static constexpr std::string_view prompt{">> "};

    while (true) {
        std::print(os, prompt);

        std::string line;
        std::getline(is, line);

        lexer::lexer lex{line};
        parser::parser p{lex};

        auto program{p.parse_program()};
        if (!p.errors.empty()) {
            for (const auto& err : p.errors) {
                std::println(os, "\t{}", err);
            }
            continue;
        }

        auto evaluated{eval::eval(program)};
        if (evaluated == nullptr) {
            continue;
        }

        std::println(os, "{}", evaluated->inspect());
        std::println(os);
    }
}

}

}
