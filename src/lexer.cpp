#include "lexer.h"
#include "helpers.h"

namespace interp {

namespace lexer {

lexer::lexer(std::string_view input) : input{input} {
    read_char();
}

token::token lexer::next_token() {
    token::token tok{};

    skip_whitespace();

    switch (ch) {
    case '=':
        if (peek_char() == '=') {
            auto c = ch;
            read_char();
            tok = token::token{token::token_type::Eq, std::string{c} + std::string{ch}};
        } else {
            tok = token::token{token::token_type::Assign, ch};
        }
        break;

    case '+':
        tok = token::token{token::token_type::Plus, ch};
        break;

    case '-':
        tok = token::token{token::token_type::Minus, ch};
        break;

    case '!':
        if (peek_char() == '=') {
            auto c = ch;
            read_char();
            tok = token::token{token::token_type::NotEq, std::string{c} + std::string{ch}};
        } else {
            tok = token::token{token::token_type::Bang, ch};
        }
        break;

    case '/':
        tok = token::token{token::token_type::Slash, ch};
        break;

    case '*':
        tok = token::token{token::token_type::Asterisk, ch};
        break;

    case '<':
        tok = token::token{token::token_type::Lt, ch};
        break;

    case '>':
        tok = token::token{token::token_type::Gt, ch};
        break;

    case ';':
        tok = token::token{token::token_type::Semicolon, ch};
        break;

    case '(':
        tok = token::token{token::token_type::Lparen, ch};
        break;

    case ')':
        tok = token::token{token::token_type::Rparen, ch};
        break;

    case ',':
        tok = token::token{token::token_type::Comma, ch};
        break;

    case '{':
        tok = token::token{token::token_type::Lbrace, ch};
        break;

    case '}':
        tok = token::token{token::token_type::Rbrace, ch};
        break;

    case 0:
        tok = token::token{token::token_type::End, ""};
        break;

    default:
        if (helpers::is_letter(ch)) {
            auto ident = read_ident();
            tok = token::token{token::lookup_ident(ident), ident};

            return tok;
        } else if (std::isdigit(ch)) {
            tok = token::token{token::token_type::Int, read_number()};

            return tok;
        } else {
            tok = token::token{token::token_type::Illegal, ch};
        }

        break;
    }

    read_char();

    return tok;
}

void lexer::read_char() {
    if (read_pos >= input.size()) {
        ch = 0;
    } else {
        ch = input[static_cast<u32>(read_pos)];
    }

    pos = read_pos;
    read_pos++;
}

i8 lexer::peek_char() {
    if (read_pos >= input.size()) {
        return 0;
    } else {
        return input[static_cast<u32>(read_pos)];
    }
}

std::string lexer::read_ident() {
    auto p = pos;

    while (helpers::is_letter(ch)) {
        read_char();
    }

    return input.substr(static_cast<u32>(p), static_cast<u32>(pos - p));
}

std::string lexer::read_number() {
    auto p = pos;

    while (std::isdigit(ch)) {
        read_char();
    }

    return input.substr(static_cast<u32>(p), static_cast<u32>(pos - p));
}

void lexer::skip_whitespace() {
    while (std::isspace(ch)) {
        read_char();
    }
}

}

}
