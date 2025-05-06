#pragma once

#include "token.h"
#include "types.h"

#include <string>

namespace interp {

namespace lexer {

class lexer {
public:
    lexer(std::string_view input);

    token::token next_token();

private:
    void read_char();
    i8 peek_char();

    std::string read_ident();
    std::string read_number();

    void skip_whitespace();

private:
    std::string input{};
    i32 pos{};
    i32 read_pos{};
    i8 ch{};
};

}

}
