#include "eval.h"
#include "ast.h"
#include "object.h"
#include <print>

namespace interp {

namespace eval {

static auto eval_statements(std::vector<std::unique_ptr<ast::statement>>& stmts) -> std::unique_ptr<object::object> {
    std::unique_ptr<object::object> result{nullptr};

    for (const auto& stmt : stmts) {
        result = eval(*stmt);
    }

    return result;
}

static auto eval_prefix_expression(std::string_view oper, const object::object& obj)
    -> std::unique_ptr<object::object> {
    if (oper == "!") {
        if (auto n{dynamic_cast<const object::boolean*>(&obj)}) {
            return std::make_unique<object::boolean>(!n->value);
        } else if (dynamic_cast<const object::null*>(&obj)) {
            return std::make_unique<object::boolean>(true);
        } else {
            return std::make_unique<object::boolean>(false);
        }
    } else if (oper == "-") {
        if (obj.type() != object::object_type::Integer) {
            return std::make_unique<object::null>();
        }

        auto n{dynamic_cast<const object::integer&>(obj)};
        n.value = -n.value;

        return std::make_unique<object::integer>(n);
    }

    return std::make_unique<object::null>();
}

static auto eval_infix_expression(std::string_view oper, const object::object& left, const object::object& right)
    -> std::unique_ptr<object::object> {
    using namespace interp;

    if (left.type() == object::object_type::Integer && right.type() == object::object_type::Integer) {
        auto& left_val{dynamic_cast<const object::integer&>(left).value};
        auto& right_val{dynamic_cast<const object::integer&>(right).value};

        if (oper == "+") {
            return std::make_unique<object::integer>(left_val + right_val);
        } else if (oper == "-") {
            return std::make_unique<object::integer>(left_val - right_val);
        } else if (oper == "*") {
            return std::make_unique<object::integer>(left_val * right_val);
        } else if (oper == "/") {
            return std::make_unique<object::integer>(left_val / right_val);
        } else if (oper == ">") {
            return std::make_unique<object::boolean>(left_val > right_val);
        } else if (oper == "<") {
            return std::make_unique<object::boolean>(left_val < right_val);
        } else if (oper == "!=") {
            return std::make_unique<object::boolean>(left_val != right_val);
        } else if (oper == "==") {
            return std::make_unique<object::boolean>(left_val == right_val);
        }
    } else if (left.type() == object::object_type::Boolean && right.type() == object::object_type::Boolean) {
        auto& left_val{dynamic_cast<const object::boolean&>(left).value};
        auto& right_val{dynamic_cast<const object::boolean&>(right).value};

        if (oper == "!=") {
            return std::make_unique<object::boolean>(left_val != right_val);
        } else if (oper == "==") {
            return std::make_unique<object::boolean>(left_val == right_val);
        }
    }

    return std::make_unique<object::null>();
}

static auto is_truthy(const object::object& obj) -> bool {
    if (auto val{dynamic_cast<const object::boolean*>(&obj)}) {
        if (val->value) {
            return true;
        } else {
            return false;
        }
    } else if (dynamic_cast<const object::null*>(&obj)) {
        return false;
    }

    return true;
}

auto eval(ast::node& node) -> std::unique_ptr<object::object> {
    if (auto n{dynamic_cast<ast::program*>(&node)}) {
        return eval_statements(n->statements);

    } else if (auto n{dynamic_cast<ast::block_statement*>(&node)}) {
        return eval_statements(n->statements);

    } else if (auto n{dynamic_cast<ast::expression_statement*>(&node)}) {
        return eval(*n->expr);

    } else if (auto n{dynamic_cast<ast::integer_literal*>(&node)}) {
        return std::make_unique<object::integer>(n->value);

    } else if (auto n{dynamic_cast<ast::boolean_expression*>(&node)}) {
        return std::make_unique<object::boolean>(n->value);

    } else if (auto n{dynamic_cast<ast::prefix_expression*>(&node)}) {
        auto right{eval(*n->right)};
        return eval_prefix_expression(n->oper, *right);

    } else if (auto n{dynamic_cast<ast::infix_expression*>(&node)}) {
        auto left{eval(*n->left)};
        auto right{eval(*n->right)};
        return eval_infix_expression(n->oper, *left, *right);

    } else if (auto n{dynamic_cast<ast::if_expression*>(&node)}) {
        auto condition{eval(*n->condition)};

        if (is_truthy(*condition)) {
            auto x = eval(*n->consequence);
            return x;
        } else if (n->alternative != nullptr) {
            return eval(*n->alternative);
        } else {
            return std::make_unique<object::null>();
        }
    }

    return nullptr;
}

}
}
