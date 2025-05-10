#pragma once

#include "token.h"
#include <memory>
#include <string>
#include <vector>

namespace interp {

namespace ast {

class node {
public:
    virtual ~node() {};

    virtual auto token_literal() const -> std::string = 0;
    virtual auto to_string() const -> std::string = 0;
};

class statement : public node {
public:
    virtual auto statement_node() const -> void = 0;
};

class expression : public node {
public:
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
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::string value{};
};

class let_statement : public statement {
public:
    let_statement(const token::token& tok) : token{tok} {}

    auto statement_node() const -> void override {};
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

    auto statement_node() const -> void override {}
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> value{};
};

class expression_statement : public statement {
public:
    expression_statement(const token::token& tok) : token{tok} {}

    auto statement_node() const -> void override {}
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

    auto expression_node() const -> void override {}
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
    infix_expression(const token::token& tok, std::string_view op, std::unique_ptr<ast::expression> l) : token{tok}, left{std::move(l)}, oper{op} {}

    auto expression_node() const -> void override {}
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

    auto statement_node() const -> void override {};
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

    auto expression_node() const -> void override {}
    auto token_literal() const -> std::string override;
    auto to_string() const -> std::string override;

public:
    token::token token{};
    std::unique_ptr<expression> condition{};
    std::unique_ptr<block_statement> consequence{};
    std::unique_ptr<block_statement> alternative{};
};

}

}
