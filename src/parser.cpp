#include "parser.h"
#include "ast.h"
#include "token.h"

#include <format>
#include <memory>

namespace interp {

namespace parser {

auto parser::no_prefix_parse_fn(token::token_type tt) -> void {
    errors.push_back(std::format("no prefix parse function found for token_type '{}'", token::get_token_type_string(tt)));
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

auto parse_prefix_expr(parser& p) -> std::unique_ptr<ast::prefix_expression> {
    ast::prefix_expression expr{p.curr_token, p.curr_token.literal};

    p.next_token();

    expr.right = p.parse_expr(ExprPrecedence::Prefix);

    return std::make_unique<ast::prefix_expression>(std::move(expr));
}

parser::parser(lexer::lexer& l) : lexer{l} {
    prefix_parser_fns[token::token_type::Ident] = parse_identifier;
    prefix_parser_fns[token::token_type::Int] = parse_integer_literal;
    prefix_parser_fns[token::token_type::Bang] = parse_prefix_expr;
    prefix_parser_fns[token::token_type::Minus] = parse_prefix_expr;

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
    std::unique_ptr<ast::let_statement> stmt{std::make_unique<ast::let_statement>(curr_token)};

    if (!expect_peek(token::token_type::Ident)) {
        return nullptr;
    }

    stmt->name = ast::identifier{curr_token, curr_token.literal};

    if (!expect_peek(token::token_type::Assign)) {
        return nullptr;
    }

    while (curr_token.type != token::token_type::Semicolon) {
        next_token();
    }

    return stmt;
}

auto parser::parse_return_stmt() -> std::unique_ptr<ast::return_statement> {
    std::unique_ptr<ast::return_statement> stmt{std::make_unique<ast::return_statement>(curr_token)};

    next_token();

    while (curr_token.type != token::token_type::Semicolon) {
        next_token();
    }

    return stmt;
}

auto parser::parse_expr_stmt() -> std::unique_ptr<ast::expression_statement> {
    ast::expression_statement stmt{curr_token};

    stmt.expression = parse_expr(ExprPrecedence::Lowest);

    if (peek_token.type == token::token_type::Semicolon) {
        next_token();
    }

    return std::make_unique<ast::expression_statement>(std::move(stmt));
}

auto parser::parse_expr(ExprPrecedence precedence) -> std::unique_ptr<ast::expression> {
    auto prefix{prefix_parser_fns[curr_token.type]};
    if (!static_cast<bool>(prefix)) {
        no_prefix_parse_fn(curr_token.type);
        return nullptr;
    }

    auto left_expr{prefix(*this)};

    return left_expr;
}

auto parser::peek_error(token::token_type t) -> void {
    errors.emplace_back(
        std::format("expected next token to be {}, got {} instead", token::get_token_type_string(t), token::get_token_type_string(peek_token.type))
    );
}

}

}
