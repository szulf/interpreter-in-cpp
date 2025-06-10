#include "token.h"
#include <utility>

namespace interp {

namespace token {

std::unordered_map<std::string_view, token_type> keywords{
    {"fn",     token_type::Function},
    {"let",    token_type::Let     },
    {"true",   token_type::True    },
    {"false",  token_type::False   },
    {"if",     token_type::If      },
    {"else",   token_type::Else    },
    {"return", token_type::Return  },
    {"while",  token_type::While   },
};

auto lookup_ident(std::string_view ident) -> token_type {
    if (keywords.contains(ident)) {
        return keywords[ident];
    }

    return token_type::Ident;
}

auto get_token_type_string(token_type t) -> std::string_view {
    switch (t) {
    case token_type::Illegal:
        return "Illegal";
    case token_type::End:
        return "End";
    case token_type::Ident:
        return "Ident";
    case token_type::Int:
        return "Int";
    case token_type::Assign:
        return "Assign";
    case token_type::Plus:
        return "Plus";
    case token_type::Minus:
        return "Minus";
    case token_type::Bang:
        return "Bang";
    case token_type::Asterisk:
        return "Asterisk";
    case token_type::Slash:
        return "Slash";
    case token_type::Lt:
        return "Lt";
    case token_type::Gt:
        return "Gt";
    case token_type::Eq:
        return "Eq";
    case token_type::NotEq:
        return "NotEq";
    case token_type::Comma:
        return "Comma";
    case token_type::Semicolon:
        return "Semicolon";
    case token_type::Lparen:
        return "Lparen";
    case token_type::Rparen:
        return "Rparen";
    case token_type::Lbrace:
        return "Lbrace";
    case token_type::Rbrace:
        return "Rbrace";
    case token_type::Function:
        return "Function";
    case token_type::Let:
        return "Let";
    case token_type::True:
        return "True";
    case token_type::False:
        return "False";
    case token_type::If:
        return "If";
    case token_type::Else:
        return "Else";
    case token_type::Return:
        return "Return";
    case token_type::String:
        return "String";
    case token_type::Lbracket:
        return "Lbracket";
    case token_type::Rbracket:
        return "Rbracket";
    case token_type::Colon:
        return "Colon";
    case token_type::While:
        return "While";
    }

    std::unreachable();
}

}

}
