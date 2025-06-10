#pragma once

#include "types.h"

#include <string>
#include <unordered_map>

namespace interp {

namespace token {

enum class token_type : u8 {
    Illegal,
    End,

    Ident,
    Int,
    String,

    Assign,
    Plus,
    Minus,
    Bang,
    Asterisk,
    Slash,

    Lt,
    Gt,

    Eq,
    NotEq,

    Comma,
    Colon,
    Semicolon,

    Lparen,
    Rparen,
    Lbrace,
    Rbrace,
    Lbracket,
    Rbracket,

    Function,
    Let,
    True,
    False,
    If,
    Else,
    Return,
    While,
    Break,
    Continue,
};

extern std::unordered_map<std::string_view, token_type> keywords;
auto lookup_ident(std::string_view ident) -> token_type;
auto get_token_type_string(token_type t) -> std::string_view;

class token {
public:
    token() {}
    token(token_type type, i8 literal) : type{type}, literal{literal} {}
    token(token_type type, std::string_view literal) : type{type}, literal{literal} {}

public:
    token_type type{};
    std::string literal{};
};

}

}
