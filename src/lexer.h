#pragma once

#include "token.h"
#include "types.h"

#include <string>

namespace interp {

namespace lexer {

class lexer {
public:
    lexer(std::string_view input);

    auto next_token() -> token::token;

private:
    auto read_char() -> void;
    i8 peek_char();

    auto read_ident() -> std::string;
    auto read_number() -> std::string;
    auto read_string() -> std::string;

    auto skip_whitespace() -> void;

private:
    std::string input{};
    usize pos{};
    usize read_pos{};
    i8 ch{};
};

}

}
