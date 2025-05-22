#include "eval.h"
#include "ast.h"
#include "object.h"
#include <print>
#include <type_traits>

namespace interp {

namespace eval {

static auto is_error(object::object* obj) -> bool {
    if (obj != nullptr) {
        return obj->type() == object::object_type::Error;
    }

    return false;
}

static auto eval_program(const ast::program& program, object::environment& env) -> std::unique_ptr<object::object> {
    std::unique_ptr<object::object> result{nullptr};

    for (const auto& stmt : program.statements) {
        result = eval(*stmt, env);
        if (result == nullptr) {
            continue;
        }

        switch (result->type()) {
        case object::object_type::ReturnValue: {
            auto ret{dynamic_cast<object::return_value*>(result.get())};
            return std::move(ret->value);
        } break;

        case object::object_type::Error: {
            if (dynamic_cast<object::error*>(result.get())) {
                return result;
            }
        } break;

        default: {
        } break;
        }
    }

    return result;
}

static auto eval_block_stmt(const ast::block_statement& block_stmt, object::environment& env)
    -> std::unique_ptr<object::object> {
    std::unique_ptr<object::object> result{nullptr};

    for (const auto& stmt : block_stmt.statements) {
        result = eval(*stmt, env);

        if (result != nullptr) {
            switch (result->type()) {
            case object::object_type::ReturnValue:
            case object::object_type::Error: {
                return result;
            } break;

            default:
                break;
            }
        }
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
            return std::make_unique<object::error>(
                std::format("unknown operator: -{}", object::get_object_type_string(obj.type()))
            );
        }

        auto n{dynamic_cast<const object::integer&>(obj)};
        n.value = -n.value;

        return std::make_unique<object::integer>(n);
    }

    return std::make_unique<object::error>(
        std::format("unknown operator: {}{}", oper, object::get_object_type_string(obj.type()))
    );
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
        } else {
            return std::make_unique<object::error>(std::format(
                "unknown operator: {} {} {}",
                object::get_object_type_string(left.type()),
                oper,
                object::get_object_type_string(right.type())
            ));
        }
    } else if (left.type() == object::object_type::Boolean && right.type() == object::object_type::Boolean) {
        auto& left_val{dynamic_cast<const object::boolean&>(left).value};
        auto& right_val{dynamic_cast<const object::boolean&>(right).value};

        if (oper == "!=") {
            return std::make_unique<object::boolean>(left_val != right_val);
        } else if (oper == "==") {
            return std::make_unique<object::boolean>(left_val == right_val);
        } else {
            return std::make_unique<object::error>(std::format(
                "unknown operator: {} {} {}",
                object::get_object_type_string(left.type()),
                oper,
                object::get_object_type_string(right.type())
            ));
        }
    }

    return std::make_unique<object::error>(std::format(
        "type mismatch: {} {} {}",
        object::get_object_type_string(left.type()),
        oper,
        object::get_object_type_string(right.type())
    ));
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

auto eval(ast::node& node, object::environment& env) -> std::unique_ptr<object::object> {
    if (auto n{dynamic_cast<ast::program*>(&node)}) {
        return eval_program(*n, env);

    } else if (auto n{dynamic_cast<ast::block_statement*>(&node)}) {
        return eval_block_stmt(*n, env);

    } else if (auto n{dynamic_cast<ast::expression_statement*>(&node)}) {
        return eval(*n->expr, env);

    } else if (auto n{dynamic_cast<ast::integer_literal*>(&node)}) {
        return std::make_unique<object::integer>(n->value);

    } else if (auto n{dynamic_cast<ast::boolean_expression*>(&node)}) {
        return std::make_unique<object::boolean>(n->value);

    } else if (auto n{dynamic_cast<ast::prefix_expression*>(&node)}) {
        auto right{eval(*n->right, env)};
        if (is_error(right.get())) {
            return right;
        }

        return eval_prefix_expression(n->oper, *right);

    } else if (auto n{dynamic_cast<ast::infix_expression*>(&node)}) {
        auto left{eval(*n->left, env)};
        if (is_error(left.get())) {
            return left;
        }

        auto right{eval(*n->right, env)};
        if (is_error(right.get())) {
            return right;
        }

        return eval_infix_expression(n->oper, *left, *right);

    } else if (auto n{dynamic_cast<ast::if_expression*>(&node)}) {
        auto condition{eval(*n->condition, env)};
        if (is_error(condition.get())) {
            return condition;
        }

        if (is_truthy(*condition)) {
            return eval(*n->consequence, env);
        } else if (n->alternative != nullptr) {
            return eval(*n->alternative, env);
        } else {
            return std::make_unique<object::null>();
        }

    } else if (auto n{dynamic_cast<ast::return_statement*>(&node)}) {
        auto val{eval(*n->value, env)};
        if (is_error(val.get())) {
            return val;
        }

        return std::make_unique<object::return_value>(std::move(val));
    } else if (auto n{dynamic_cast<ast::let_statement*>(&node)}) {
        auto val{eval(*n->value, env)};
        if (is_error(val.get())) {
            return val;
        }
        env.set(n->name.value, std::move(val));

    } else if (auto n{dynamic_cast<ast::identifier*>(&node)}) {
        try {
            auto& val{env.get(n->value)};
            return val->clone();
        } catch (const std::exception& e) {
            return std::make_unique<object::error>(std::format("identifier not found: {}", n->value));
        }
    }

    return nullptr;
}
}

}
