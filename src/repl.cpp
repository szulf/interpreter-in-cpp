#include "repl.h"
#include "lexer.h"

#include <iostream>
#include <string_view>

namespace interp {

namespace repl {

void start(std::istream& is, std::ostream& os) {
    static constexpr std::string_view prompt{">> "};

    while (true) {
        std::println(os, prompt);

        std::string line;
        std::getline(is, line);

        lexer::lexer lex{line};

        for (auto tok = lex.next_token(); tok.type != token::token_type::End; tok = lex.next_token()) {
            std::println(os, "{} {}", static_cast<i32>(tok.type), tok.literal);
        }
    }
}

}

}
