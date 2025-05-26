#include "parser.h"
#include "ast.h"
#include "token.h"

#include <format>
#include <memory>
#include <print>
#include <stdexcept>

namespace interp {

namespace parser {

std::unordered_map<token::token_type, expr_precedence> parser::precedences{
    {token::token_type::Eq,       expr_precedence::Equals     },
    {token::token_type::NotEq,    expr_precedence::Equals     },
    {token::token_type::Lt,       expr_precedence::LessGreater},
    {token::token_type::Gt,       expr_precedence::LessGreater},
    {token::token_type::Plus,     expr_precedence::Sum        },
    {token::token_type::Minus,    expr_precedence::Sum        },
    {token::token_type::Slash,    expr_precedence::Product    },
    {token::token_type::Asterisk, expr_precedence::Product    },
    {token::token_type::Lparen,   expr_precedence::Call       },
};

auto parser::no_prefix_parse_fn(token::token_type tt) -> void {
    errors.push_back(std::format("no prefix parse function found for token_type '{}'", token::get_token_type_string(tt))
    );
}

auto parse_identifier(parser& p) -> std::unique_ptr<ast::identifier> {
    return std::make_unique<ast::identifier>(p.curr_token, p.curr_token.literal);
}

auto parse_integer_literal(parser& p) -> std::unique_ptr<ast::integer_literal> {
    i64 value = 0;
    try {
        value = std::stol(p.curr_token.literal);
    } catch (const std::exception& e) {
        p.errors.push_back(std::format("couldnt parse {} as integer", p.curr_token.literal));
        return nullptr;
    }

    return std::make_unique<ast::integer_literal>(p.curr_token, value);
}

auto parse_prefix_expression(parser& p) -> std::unique_ptr<ast::prefix_expression> {
    auto expr = std::make_unique<ast::prefix_expression>(p.curr_token, p.curr_token.literal);

    p.next_token();

    expr->right = p.parse_expr(expr_precedence::Prefix);

    return expr;
}

auto parse_infix_expression(std::unique_ptr<ast::expression> left, parser& p)
    -> std::unique_ptr<ast::infix_expression> {
    auto expr = std::make_unique<ast::infix_expression>(p.curr_token, p.curr_token.literal, std::move(left));

    auto precedence{p.curr_precedence()};
    p.next_token();
    expr->right = p.parse_expr(precedence);

    return expr;
}

auto parse_boolean_expression(parser& p) -> std::unique_ptr<ast::boolean_expression> {
    return std::make_unique<ast::boolean_expression>(p.curr_token, p.curr_token.type == token::token_type::True);
}

auto parse_grouped_expression(parser& p) -> std::unique_ptr<ast::expression> {
    p.next_token();

    auto expr{p.parse_expr(expr_precedence::Lowest)};

    if (!p.expect_peek(token::token_type::Rparen)) {
        return nullptr;
    }

    return expr;
}

auto parse_if_expression(parser& p) -> std::unique_ptr<ast::if_expression> {
    auto expr{std::make_unique<ast::if_expression>(p.curr_token)};

    if (!p.expect_peek(token::token_type::Lparen)) {
        return nullptr;
    }

    p.next_token();
    expr->condition = p.parse_expr(expr_precedence::Lowest);

    if (!p.expect_peek(token::token_type::Rparen)) {
        return nullptr;
    }

    if (!p.expect_peek(token::token_type::Lbrace)) {
        return nullptr;
    }

    expr->consequence = p.parse_block_stmt();

    if (p.peek_token.type != token::token_type::Else) {
        return expr;
    }

    p.next_token();

    if (!p.expect_peek(token::token_type::Lbrace)) {
        return nullptr;
    }

    expr->alternative = p.parse_block_stmt();

    return expr;
}

auto parse_fn_expression(parser& p) -> std::unique_ptr<ast::fn_expression> {
    auto expr{std::make_unique<ast::fn_expression>(p.curr_token)};

    if (!p.expect_peek(token::token_type::Lparen)) {
        return nullptr;
    }

    expr->parameters = p.parse_fn_parameters();

    if (!p.expect_peek(token::token_type::Lbrace)) {
        return nullptr;
    }

    expr->body = p.parse_block_stmt();

    return expr;
}

auto parse_call_expression(std::unique_ptr<ast::expression> left, parser& p) -> std::unique_ptr<ast::call_expression> {
    auto expr{std::make_unique<ast::call_expression>(p.curr_token, std::move(left))};

    expr->arguments = p.parse_call_arguments();

    return expr;
}

parser::parser(lexer::lexer& l) : lexer{l} {
    prefix_parser_fns[token::token_type::Ident] = parse_identifier;
    prefix_parser_fns[token::token_type::Int] = parse_integer_literal;
    prefix_parser_fns[token::token_type::Bang] = parse_prefix_expression;
    prefix_parser_fns[token::token_type::Minus] = parse_prefix_expression;
    prefix_parser_fns[token::token_type::True] = parse_boolean_expression;
    prefix_parser_fns[token::token_type::False] = parse_boolean_expression;
    prefix_parser_fns[token::token_type::Lparen] = parse_grouped_expression;
    prefix_parser_fns[token::token_type::If] = parse_if_expression;
    prefix_parser_fns[token::token_type::Function] = parse_fn_expression;

    infix_parser_fns[token::token_type::Eq] = parse_infix_expression;
    infix_parser_fns[token::token_type::NotEq] = parse_infix_expression;
    infix_parser_fns[token::token_type::Lt] = parse_infix_expression;
    infix_parser_fns[token::token_type::Gt] = parse_infix_expression;
    infix_parser_fns[token::token_type::Plus] = parse_infix_expression;
    infix_parser_fns[token::token_type::Minus] = parse_infix_expression;
    infix_parser_fns[token::token_type::Slash] = parse_infix_expression;
    infix_parser_fns[token::token_type::Asterisk] = parse_infix_expression;
    infix_parser_fns[token::token_type::Lparen] = parse_call_expression;

    next_token();
    next_token();
}

auto parser::parse_program() -> ast::program {
    ast::program program{};

    while (curr_token.type != token::token_type::End) {
        auto stmt{parse_stmt()};
        if (stmt != nullptr) {
            program.statements.emplace_back(std::move(stmt));
        }

        next_token();
    }

    return program;
}

auto parser::next_token() -> void {
    curr_token = peek_token;
    peek_token = lexer.next_token();
}

auto parser::expect_peek(token::token_type tok) -> bool {
    if (peek_token.type == tok) {
        next_token();

        return true;
    }

    peek_error(tok);

    return false;
}

auto parser::parse_stmt() -> std::unique_ptr<ast::statement> {
    switch (curr_token.type) {
    case token::token_type::Let:
        return parse_let_stmt();

    case token::token_type::Return:
        return parse_return_stmt();

    default:
        return parse_expr_stmt();
    }
}

auto parser::parse_let_stmt() -> std::unique_ptr<ast::let_statement> {
    auto stmt{std::make_unique<ast::let_statement>(curr_token)};

    if (!expect_peek(token::token_type::Ident)) {
        return nullptr;
    }

    stmt->name = ast::identifier{curr_token, curr_token.literal};

    if (!expect_peek(token::token_type::Assign)) {
        return nullptr;
    }

    next_token();

    stmt->value = parse_expr(expr_precedence::Lowest);

    if (peek_token.type == token::token_type::Semicolon) {
        next_token();
    }

    return stmt;
}

auto parser::parse_return_stmt() -> std::unique_ptr<ast::return_statement> {
    auto stmt{std::make_unique<ast::return_statement>(curr_token)};

    next_token();

    stmt->value = parse_expr(expr_precedence::Lowest);

    if (peek_token.type == token::token_type::Semicolon) {
        next_token();
    }

    return stmt;
}

auto parser::parse_expr_stmt() -> std::unique_ptr<ast::expression_statement> {
    ast::expression_statement stmt{curr_token};

    stmt.expr = parse_expr(expr_precedence::Lowest);

    if (peek_token.type == token::token_type::Semicolon) {
        next_token();
    }

    return std::make_unique<ast::expression_statement>(std::move(stmt));
}

auto parser::parse_expr(expr_precedence precedence) -> std::unique_ptr<ast::expression> {
    auto prefix{prefix_parser_fns[curr_token.type]};
    if (!static_cast<bool>(prefix)) {
        no_prefix_parse_fn(curr_token.type);
        return nullptr;
    }

    auto left_expr{prefix(*this)};
    while (peek_token.type != token::token_type::Semicolon && precedence < peek_precedence()) {
        auto infix{infix_parser_fns[peek_token.type]};
        if (!static_cast<bool>(infix)) {
            return left_expr;
        }

        next_token();

        left_expr = infix(std::move(left_expr), *this);
    }

    return left_expr;
}

auto parser::parse_block_stmt() -> std::unique_ptr<ast::block_statement> {
    auto block{std::make_unique<ast::block_statement>(curr_token)};

    next_token();

    while (curr_token.type != token::token_type::Rbrace && curr_token.type != token::token_type::End) {
        auto stmt{parse_stmt()};
        if (stmt != nullptr) {
            block->statements.emplace_back(std::move(stmt));
        }

        next_token();
    }

    return block;
}

auto parser::parse_fn_parameters() -> std::vector<std::unique_ptr<ast::expression>> {
    std::vector<std::unique_ptr<ast::expression>> parameters{};

    next_token();

    if (curr_token.type == token::token_type::Rparen) {
        return parameters;
    }

    parameters.emplace_back(std::make_unique<ast::identifier>(curr_token, curr_token.literal));

    while (peek_token.type == token::token_type::Comma) {
        next_token();
        next_token();

        parameters.emplace_back(std::make_unique<ast::identifier>(curr_token, curr_token.literal));
    }

    if (!expect_peek(token::token_type::Rparen)) {
        return {};
    }

    return parameters;
}

auto parser::parse_call_arguments() -> std::vector<std::unique_ptr<ast::expression>> {
    std::vector<std::unique_ptr<ast::expression>> parameters{};

    next_token();

    if (curr_token.type == token::token_type::Rparen) {
        return parameters;
    }

    parameters.emplace_back(parse_expr(expr_precedence::Lowest));

    while (peek_token.type == token::token_type::Comma) {
        next_token();
        next_token();

        parameters.emplace_back(parse_expr(expr_precedence::Lowest));
    }

    if (!expect_peek(token::token_type::Rparen)) {
        return {};
    }

    return parameters;
}

auto parser::peek_error(token::token_type t) -> void {
    errors.emplace_back(std::format(
        "expected next token to be {}, got {} instead",
        token::get_token_type_string(t),
        token::get_token_type_string(peek_token.type)
    ));
}

auto parser::curr_precedence() -> expr_precedence {
    try {
        return precedences.at(curr_token.type);
    } catch (const std::out_of_range&) {
        return expr_precedence::Lowest;
    }
}

auto parser::peek_precedence() -> expr_precedence {
    try {
        return precedences.at(peek_token.type);
    } catch (const std::out_of_range&) {
        return expr_precedence::Lowest;
    }
}

}

}
