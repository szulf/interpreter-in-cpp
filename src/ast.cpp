#include "ast.h"
#include <memory>
#include <sstream>

namespace interp {

namespace ast {

auto program::token_literal() const -> std::string {
    if (statements.empty()) {
        return {};
    }

    return statements[0]->token_literal();
}

auto program::to_string() const -> std::string {
    std::stringstream ss{};

    for (const auto& s : statements) {
        ss << s->to_string();
    }

    return ss.str();
}

auto identifier::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<identifier>(*this);
}

auto identifier::token_literal() const -> std::string {
    return token.literal;
}

auto identifier::to_string() const -> std::string {
    return value;
}

auto let_statement::clone() const -> std::unique_ptr<statement> {
    return std::make_unique<let_statement>(*this);
}

auto let_statement::token_literal() const -> std::string {
    return token.literal;
}

auto let_statement::to_string() const -> std::string {
    std::stringstream ss{};

    ss << token_literal() << " ";
    ss << name.to_string();
    ss << " = ";

    if (value != nullptr) {
        ss << value->to_string();
    }

    ss << ";";

    return ss.str();
}

auto return_statement::clone() const -> std::unique_ptr<statement> {
    return std::make_unique<return_statement>(*this);
}

auto return_statement::token_literal() const -> std::string {
    return token.literal;
}

auto return_statement::to_string() const -> std::string {
    std::stringstream ss{};

    ss << token_literal() << " ";

    if (value != nullptr) {
        ss << value->to_string();
    }

    ss << ";";

    return ss.str();
}

auto expression_statement::clone() const -> std::unique_ptr<statement> {
    return std::make_unique<expression_statement>(*this);
}

auto expression_statement::token_literal() const -> std::string {
    return token.literal;
}

auto expression_statement::to_string() const -> std::string {
    if (expr != nullptr) {
        return expr->to_string();
    }

    return {};
}

auto integer_literal::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<integer_literal>(*this);
}

auto integer_literal::token_literal() const -> std::string {
    return token.literal;
}

auto integer_literal::to_string() const -> std::string {
    return token.literal;
}

auto prefix_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<prefix_expression>(*this);
}

auto prefix_expression::token_literal() const -> std::string {
    return token.literal;
}

auto prefix_expression::to_string() const -> std::string {
    return std::format("({}{})", oper, right->to_string());
}

auto infix_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<infix_expression>(*this);
}

auto infix_expression::token_literal() const -> std::string {
    return token.literal;
}

auto infix_expression::to_string() const -> std::string {
    return std::format("({} {} {})", left->to_string(), oper, right->to_string());
}

auto boolean_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<boolean_expression>(*this);
}

auto boolean_expression::token_literal() const -> std::string {
    return token.literal;
}

auto boolean_expression::to_string() const -> std::string {
    return token.literal;
}

block_statement::block_statement(const block_statement& other) : token{other.token} {
    for (const auto& stmt : other.statements) {
        statements.emplace_back(stmt->clone());
    }
}

auto block_statement::clone() const -> std::unique_ptr<statement> {
    return std::make_unique<block_statement>(*this);
}

auto block_statement::token_literal() const -> std::string {
    return token.literal;
}

auto block_statement::to_string() const -> std::string {
    std::stringstream ss{};

    for (const auto& stmt : statements) {
        ss << stmt->to_string();
    }

    return ss.str();
}

if_expression::if_expression(const if_expression& other)
    : token{other.token}, condition{other.condition->clone()}, consequence{other.consequence->clone()} {
    if (other.alternative) {
        alternative = other.alternative->clone();
    }
}

auto if_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<if_expression>(*this);
}

auto if_expression::token_literal() const -> std::string {
    return token.literal;
}

auto if_expression::to_string() const -> std::string {
    std::stringstream ss{};

    ss << "if" << condition->to_string();
    ss << " " << consequence->to_string();
    if (alternative != nullptr) {
        ss << "else " << alternative->to_string();
    }

    return ss.str();
}

fn_expression::fn_expression(const fn_expression& other) : token{other.token}, body{other.body->clone()} {
    for (const auto& param : other.parameters) {
        parameters.push_back(param->clone());
    }
}

auto fn_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<fn_expression>(*this);
}

auto fn_expression::token_literal() const -> std::string {
    return token.literal;
}

auto fn_expression::to_string() const -> std::string {
    std::stringstream ss{};

    ss << token_literal() << "(";

    for (u32 i{0}; i < parameters.size(); i++) {
        ss << parameters[i];
        if (i != parameters.size() - 1) {
            ss << ", ";
        }
    }

    ss << ")" << body->to_string();

    return ss.str();
}

call_expression::call_expression(const call_expression& other) : token{other.token}, fn{other.fn->clone()} {
    for (const auto& arg : other.arguments) {
        arguments.emplace_back(arg->clone());
    }
}

auto call_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<call_expression>(*this);
}

auto call_expression::token_literal() const -> std::string {
    return token.literal;
}

auto call_expression::to_string() const -> std::string {
    std::stringstream ss{};

    ss << fn->to_string() << "(";

    for (u32 i{0}; i < arguments.size(); i++) {
        ss << arguments[i]->to_string();
        if (i != arguments.size() - 1) {
            ss << ", ";
        }
    }

    ss << ")";

    return ss.str();
}

auto string_literal::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<string_literal>(*this);
}

auto string_literal::token_literal() const -> std::string {
    return token.literal;
}

auto string_literal::to_string() const -> std::string {
    return token.literal;
}

array_literal::array_literal(const array_literal& other) : token{other.token} {
    for (const auto& elem : other.elements) {
        elements.emplace_back(elem->clone());
    }
}

auto array_literal::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<array_literal>(*this);
}

auto array_literal::token_literal() const -> std::string {
    return token.literal;
}

auto array_literal::to_string() const -> std::string {
    std::stringstream ss{};

    ss << "[";
    for (const auto& elem : elements) {
        ss << elem->to_string();
        if (elem != elements.back()) {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str();
}

auto index_expression::clone() const -> std::unique_ptr<expression> {
    return std::make_unique<index_expression>(*this);
}

auto index_expression::token_literal() const -> std::string {
    return token.literal;
}

auto index_expression::to_string() const -> std::string {
    return std::format("({}[{}])", left->to_string(), index->to_string());
}

}

}
