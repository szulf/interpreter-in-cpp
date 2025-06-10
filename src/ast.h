#pragma once

#include "token.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace interp {

namespace ast {

class node {
public:
    virtual ~node() = default;

    virtual auto token_literal() const -> std::string = 0;
    virtual auto to_string() const -> std::string = 0;
};

class statement : public node {
public:
    virtual auto clone() const -> std::unique_ptr<statement> = 0;
    virtual auto statement_node() const -> void = 0;
};

class expression : public node {
public:
    virtual auto clone() const -> std::unique_ptr<expression> = 0;
    virtual auto expression_node() const -> void = 0;
};

class program : public node {
public:
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    std::vector<std::unique_ptr<statement>> statements{};
};

class identifier : public expression {
public:
    identifier() {}
    identifier(const token::token& tok, std::string_view val) : token{tok}, value{val} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::string value{};
};

class let_statement : public statement {
public:
    let_statement(const token::token& tok) : token{tok} {}
    let_statement(const let_statement& other) : token{other.token}, name{other.name}, value{other.value->clone()} {}

    auto statement_node() const -> void override {};
    auto clone() const -> std::unique_ptr<statement> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    identifier name{};
    std::unique_ptr<expression> value{};
};

class return_statement : public statement {
public:
    return_statement(const token::token& tok) : token{tok} {}
    return_statement(const return_statement& other) : token{other.token}, value{other.value->clone()} {}

    auto statement_node() const -> void override {}
    auto clone() const -> std::unique_ptr<statement> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> value{};
};

class expression_statement : public statement {
public:
    expression_statement(const token::token& tok) : token{tok} {}
    expression_statement(const expression_statement& other) : token{other.token}, expr{other.expr->clone()} {}

    auto statement_node() const -> void override {}
    auto clone() const -> std::unique_ptr<statement> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> expr{};
};

class integer_literal : public expression {
public:
    integer_literal() {}
    integer_literal(const token::token& tok, i64 val) : token{tok}, value{val} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    i64 value{};
};

class prefix_expression : public expression {
public:
    prefix_expression() {}
    prefix_expression(const token::token& tok, std::string_view op) : token{tok}, oper{op} {}
    prefix_expression(const prefix_expression& other)
        : token{other.token}, oper{other.oper}, right{other.right->clone()} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::string oper{};
    std::unique_ptr<expression> right{};
};

class infix_expression : public expression {
public:
    infix_expression() {}
    infix_expression(const token::token& tok, std::string_view op, std::unique_ptr<ast::expression> l)
        : token{tok}, left{std::move(l)}, oper{op} {}
    infix_expression(const infix_expression& other)
        : token{other.token}, left{other.left->clone()}, oper{other.oper}, right{other.right->clone()} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> left{};
    std::string oper{};
    std::unique_ptr<expression> right{};
};

class boolean_expression : public expression {
public:
    boolean_expression() {}
    boolean_expression(const token::token& tok, bool val) : token{tok}, value{val} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    bool value{};
};

class block_statement : public statement {
public:
    block_statement() {}
    block_statement(const token::token& tok) : token{tok} {}
    block_statement(const block_statement& tok);

    auto statement_node() const -> void override {};
    auto clone() const -> std::unique_ptr<statement> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::vector<std::unique_ptr<statement>> statements{};
};

class if_expression : public expression {
public:
    if_expression() {}
    if_expression(const token::token& tok) : token{tok} {}
    if_expression(const if_expression& other);

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> condition{};
    std::unique_ptr<statement> consequence{};
    std::unique_ptr<statement> alternative{};
};

class fn_expression : public expression {
public:
    fn_expression() {}
    fn_expression(const token::token& tok) : token{tok} {}
    fn_expression(const fn_expression& other);

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::vector<std::unique_ptr<expression>> parameters{};
    std::unique_ptr<statement> body{};
};

class call_expression : public expression {
public:
    call_expression() {}
    call_expression(const token::token& tok, std::unique_ptr<expression> fn) : token{tok}, fn{std::move(fn)} {}
    call_expression(const call_expression& other);

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> fn{};
    std::vector<std::unique_ptr<expression>> arguments{};
};

class string_literal : public expression {
public:
    string_literal() {}
    string_literal(const token::token& tok, const std::string& val) : token{tok}, value{val} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::string value{};
};

class array_literal : public expression {
public:
    array_literal() {}
    array_literal(const token::token& tok) : token{tok} {}
    array_literal(const array_literal& other);

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::vector<std::unique_ptr<expression>> elements{};
};

class index_expression : public expression {
public:
    index_expression() {}
    index_expression(const token::token& tok, std::unique_ptr<expression> l) : token{tok}, left{std::move(l)} {}
    index_expression(const index_expression& other)
        : token{other.token}, left{other.left->clone()}, index{other.index->clone()} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> left{};
    std::unique_ptr<expression> index{};
};

class hash_literal : public expression {
public:
    hash_literal() {}
    hash_literal(const token::token& tok) : token{tok} {}
    hash_literal(const hash_literal& other);

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unordered_map<std::unique_ptr<expression>, std::unique_ptr<expression>> pairs{};
};

class assign_expression : public expression {
public:
    assign_expression() {}
    assign_expression(const token::token& tok, std::unique_ptr<expression> n) : token{tok}, name{std::move(n)} {}
    assign_expression(const assign_expression& other)
        : token{other.token}, name{other.name->clone()}, value{other.value->clone()} {}

    auto expression_node() const -> void override {}
    auto clone() const -> std::unique_ptr<expression> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> name{};
    std::unique_ptr<expression> value{};
};

class while_statement : public statement {
public:
    while_statement(const token::token& tok) : token{tok} {}
    while_statement(const while_statement& other)
        : token{other.token}, condition{other.condition->clone()}, body{other.body->clone()} {}

    auto statement_node() const -> void override {};
    auto clone() const -> std::unique_ptr<statement> override;
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> condition{};
    std::unique_ptr<statement> body{};
};

}

}
