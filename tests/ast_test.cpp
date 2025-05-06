#include <gtest/gtest.h>

#include "../src/ast.h"

TEST(ast, let_string) {
    using namespace interp;

    ast::program p{};

    token::token let_tok{};
    let_tok.literal = "let";
    let_tok.type = token::token_type::Let;

    token::token let_ident_tok{};
    let_ident_tok.literal = "myVar";
    let_ident_tok.type = token::token_type::Ident;

    ast::identifier let_ident{};
    let_ident.token = let_ident_tok;
    let_ident.value = "myVar";

    token::token ident_tok{};
    ident_tok.type = token::token_type::Ident;
    ident_tok.literal = "anotherVal";

    ast::identifier ident{};
    ident.token = ident_tok;
    ident.value = "anotherVal";

    ast::let_statement let{let_tok};
    let.name = let_ident;
    let.value = std::make_unique<ast::identifier>(ident);

    p.statements.push_back(std::make_unique<ast::let_statement>(std::move(let)));

    ASSERT_STREQ(p.to_string().c_str(), "let myVar = anotherVal;");
}
