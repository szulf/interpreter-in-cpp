#pragma once

#include "ast.h"
#include "lexer.h"
#include "token.h"

#include <functional>
#include <memory>
#include <unordered_map>

namespace interp {

namespace parser {

class parser;

using prefix_parser_fn = std::function<std::unique_ptr<ast::expression>(parser& p)>;
using infix_parser_fn = std::function<std::unique_ptr<ast::expression>(std::unique_ptr<ast::expression>, parser& p)>;

enum class expr_precedence {
    Lowest,
    Equals,
    LessGreater,
    Sum,
    Product,
    Prefix,
    Call,
};

class parser {
public:
    parser(lexer::lexer& lexer);

    auto parse_program() -> ast::program;

private:
    auto next_token() -> void;
    auto expect_peek(token::token_type tok) -> bool;

    auto parse_stmt() -> std::unique_ptr<ast::statement>;
    auto parse_let_stmt() -> std::unique_ptr<ast::let_statement>;
    auto parse_return_stmt() -> std::unique_ptr<ast::return_statement>;
    auto parse_expr_stmt() -> std::unique_ptr<ast::expression_statement>;
    auto parse_expr(expr_precedence precedence) -> std::unique_ptr<ast::expression>;
    auto parse_block_stmt() -> std::unique_ptr<ast::block_statement>;
    auto parse_fn_parameters() -> std::vector<std::unique_ptr<ast::expression>>;
    auto parse_expression_list(token::token_type tok_type) -> std::vector<std::unique_ptr<ast::expression>>;

    auto peek_error(token::token_type t) -> void;

    auto no_prefix_parse_fn(token::token_type tt) -> void;

    friend auto parse_prefix_expression(parser& p) -> std::unique_ptr<ast::prefix_expression>;
    friend auto parse_infix_expression(std::unique_ptr<ast::expression> left, parser& p)
        -> std::unique_ptr<ast::infix_expression>;
    friend auto parse_grouped_expression(parser& p) -> std::unique_ptr<ast::expression>;
    friend auto parse_if_expression(parser& p) -> std::unique_ptr<ast::if_expression>;
    friend auto parse_fn_expression(parser& p) -> std::unique_ptr<ast::fn_expression>;
    friend auto parse_call_expression(std::unique_ptr<ast::expression> left, parser& p)
        -> std::unique_ptr<ast::call_expression>;
    friend auto parse_array_literal(parser& p) -> std::unique_ptr<ast::expression>;

    auto curr_precedence() -> expr_precedence;
    auto peek_precedence() -> expr_precedence;

public:
    lexer::lexer& lexer;

    token::token curr_token{};
    token::token peek_token{};

    std::vector<std::string> errors{};

    std::unordered_map<token::token_type, prefix_parser_fn> prefix_parser_fns{};
    std::unordered_map<token::token_type, infix_parser_fn> infix_parser_fns{};

    static std::unordered_map<token::token_type, expr_precedence> precedences;
};
}

}
