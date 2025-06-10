#include <gtest/gtest.h>

#include "lexer.h"
#include "token.h"

TEST(lexer, next_token_1) {
    using namespace interp;

    static constexpr std::string_view input{"=+(){},;"};

    std::array tests{
        token::token{token::token_type::Assign,    "="},
        token::token{token::token_type::Plus,      "+"},
        token::token{token::token_type::Lparen,    "("},
        token::token{token::token_type::Rparen,    ")"},
        token::token{token::token_type::Lbrace,    "{"},
        token::token{token::token_type::Rbrace,    "}"},
        token::token{token::token_type::Comma,     ","},
        token::token{token::token_type::Semicolon, ";"},

        token::token{token::token_type::End,       "" },
    };

    lexer::lexer l{input};

    for (const auto& expected : tests) {
        auto tok = l.next_token();

        ASSERT_EQ(expected.type, tok.type);
        ASSERT_EQ(expected.literal, tok.literal);
    }
}

TEST(lexer, next_token_2) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    let five = 5;
    let ten = 10;

    let add = fn(x, y) {
        x + y;
    };

    let result = add(five, ten);
    )"};

    std::array tests{
        token::token{token::token_type::Let,       "let"   },
        token::token{token::token_type::Ident,     "five"  },
        token::token{token::token_type::Assign,    "="     },
        token::token{token::token_type::Int,       "5"     },
        token::token{token::token_type::Semicolon, ";"     },

        token::token{token::token_type::Let,       "let"   },
        token::token{token::token_type::Ident,     "ten"   },
        token::token{token::token_type::Assign,    "="     },
        token::token{token::token_type::Int,       "10"    },
        token::token{token::token_type::Semicolon, ";"     },

        token::token{token::token_type::Let,       "let"   },
        token::token{token::token_type::Ident,     "add"   },
        token::token{token::token_type::Assign,    "="     },
        token::token{token::token_type::Function,  "fn"    },
        token::token{token::token_type::Lparen,    "("     },
        token::token{token::token_type::Ident,     "x"     },
        token::token{token::token_type::Comma,     ","     },
        token::token{token::token_type::Ident,     "y"     },
        token::token{token::token_type::Rparen,    ")"     },
        token::token{token::token_type::Lbrace,    "{"     },

        token::token{token::token_type::Ident,     "x"     },
        token::token{token::token_type::Plus,      "+"     },
        token::token{token::token_type::Ident,     "y"     },
        token::token{token::token_type::Semicolon, ";"     },

        token::token{token::token_type::Rbrace,    "}"     },
        token::token{token::token_type::Semicolon, ";"     },

        token::token{token::token_type::Let,       "let"   },
        token::token{token::token_type::Ident,     "result"},
        token::token{token::token_type::Assign,    "="     },
        token::token{token::token_type::Ident,     "add"   },
        token::token{token::token_type::Lparen,    "("     },
        token::token{token::token_type::Ident,     "five"  },
        token::token{token::token_type::Comma,     ","     },
        token::token{token::token_type::Ident,     "ten"   },
        token::token{token::token_type::Rparen,    ")"     },
        token::token{token::token_type::Semicolon, ";"     },

        token::token{token::token_type::End,       ""      },
    };

    lexer::lexer l{input};

    for (const auto& expected : tests) {
        auto tok = l.next_token();

        ASSERT_EQ(expected.literal, tok.literal);
        ASSERT_EQ(expected.type, tok.type);
    }
}

TEST(lexer, next_token_3) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    let five = 5;
    let ten = 10;

    let add = fn(x, y) {
      x + y;
    };

    let result = add(five, ten);
    !-/*5;
    5 < 10 > 5;

    if (5 < 10) {
        return true;
    } else {
        return false;
    }

    10 == 10;
    10 != 9;

    "foobar"
    "foo bar"
    [1, 2];
    {"foo": "bar"}

    while (x > 5)
    )"};

    std::array tests{
        token::token{token::token_type::Let,       "let"    },
        token::token{token::token_type::Ident,     "five"   },
        token::token{token::token_type::Assign,    "="      },
        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Let,       "let"    },
        token::token{token::token_type::Ident,     "ten"    },
        token::token{token::token_type::Assign,    "="      },
        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Let,       "let"    },
        token::token{token::token_type::Ident,     "add"    },
        token::token{token::token_type::Assign,    "="      },
        token::token{token::token_type::Function,  "fn"     },
        token::token{token::token_type::Lparen,    "("      },
        token::token{token::token_type::Ident,     "x"      },
        token::token{token::token_type::Comma,     ","      },
        token::token{token::token_type::Ident,     "y"      },
        token::token{token::token_type::Rparen,    ")"      },
        token::token{token::token_type::Lbrace,    "{"      },

        token::token{token::token_type::Ident,     "x"      },
        token::token{token::token_type::Plus,      "+"      },
        token::token{token::token_type::Ident,     "y"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Rbrace,    "}"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Let,       "let"    },
        token::token{token::token_type::Ident,     "result" },
        token::token{token::token_type::Assign,    "="      },
        token::token{token::token_type::Ident,     "add"    },
        token::token{token::token_type::Lparen,    "("      },
        token::token{token::token_type::Ident,     "five"   },
        token::token{token::token_type::Comma,     ","      },
        token::token{token::token_type::Ident,     "ten"    },
        token::token{token::token_type::Rparen,    ")"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Bang,      "!"      },
        token::token{token::token_type::Minus,     "-"      },
        token::token{token::token_type::Slash,     "/"      },
        token::token{token::token_type::Asterisk,  "*"      },
        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Lt,        "<"      },
        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::Gt,        ">"      },
        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::If,        "if"     },
        token::token{token::token_type::Lparen,    "("      },
        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Lt,        "<"      },
        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::Rparen,    ")"      },
        token::token{token::token_type::Lbrace,    "{"      },

        token::token{token::token_type::Return,    "return" },
        token::token{token::token_type::True,      "true"   },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Rbrace,    "}"      },
        token::token{token::token_type::Else,      "else"   },
        token::token{token::token_type::Lbrace,    "{"      },
        token::token{token::token_type::Return,    "return" },
        token::token{token::token_type::False,     "false"  },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Rbrace,    "}"      },

        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::Eq,        "=="     },
        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Int,       "10"     },
        token::token{token::token_type::NotEq,     "!="     },
        token::token{token::token_type::Int,       "9"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::String,    "foobar" },
        token::token{token::token_type::String,    "foo bar"},

        token::token{token::token_type::Lbracket,  "["      },
        token::token{token::token_type::Int,       "1"      },
        token::token{token::token_type::Comma,     ","      },
        token::token{token::token_type::Int,       "2"      },
        token::token{token::token_type::Rbracket,  "]"      },
        token::token{token::token_type::Semicolon, ";"      },

        token::token{token::token_type::Lbrace,    "{"      },
        token::token{token::token_type::String,    "foo"    },
        token::token{token::token_type::Colon,     ":"      },
        token::token{token::token_type::String,    "bar"    },
        token::token{token::token_type::Rbrace,    "}"      },

        token::token{token::token_type::While,     "while"  },
        token::token{token::token_type::Lparen,    "("      },
        token::token{token::token_type::Ident,     "x"      },
        token::token{token::token_type::Gt,        ">"      },
        token::token{token::token_type::Int,       "5"      },
        token::token{token::token_type::Rparen,    ")"      },

        token::token{token::token_type::End,       ""       },
    };

    lexer::lexer l{input};

    for (const auto& expected : tests) {
        auto tok = l.next_token();

        ASSERT_EQ(expected.literal, tok.literal);
        ASSERT_EQ(expected.type, tok.type);
    }
}
