#include "eval.h"
#include "ast.h"
#include "object.h"
#include <cassert>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

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

static auto eval_expressions(const std::vector<std::unique_ptr<ast::expression>>& exprs, object::environment& env)
    -> std::vector<std::unique_ptr<object::object>> {
    std::vector<std::unique_ptr<object::object>> ret{};

    for (const auto& expr : exprs) {
        auto evaluated{eval(*expr, env)};
        if (is_error(evaluated.get())) {
            ret.clear();
            ret.emplace_back(std::move(evaluated));
            return ret;
        }

        ret.emplace_back(std::move(evaluated));
    }

    return ret;
}

static auto apply_function(object::object& function, std::vector<std::unique_ptr<object::object>> args)
    -> std::unique_ptr<object::object> {
    auto& fn{dynamic_cast<object::function&>(function)};

    auto env{std::make_unique<object::environment>(&fn.env_outer)};
    for (const auto& [param, arg] : std::ranges::zip_view(fn.parameters, args)) {
        env->set(dynamic_cast<const ast::identifier&>(*param).value, std::move(arg));
    }

    auto evaluated{eval(*fn.body, *env)};

    if (dynamic_cast<object::function*>(evaluated.get())) {
        fn.env_outer.envs_inner.push_back(std::move(env));
    }

    if (auto val{dynamic_cast<object::return_value*>(evaluated.get())}) {
        if (dynamic_cast<object::function*>(val->value.get())) {
            fn.env_outer.envs_inner.push_back(std::move(env));
        }

        return std::move(val->value);
    }

    return evaluated;
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
        auto* val{env.get(n->value)};
        if (val && *val) {
            return (*val)->clone();
        }

        return std::make_unique<object::error>(std::format("identifier not found: {}", n->value));

    } else if (auto n{dynamic_cast<ast::fn_expression*>(&node)}) {
        return std::make_unique<object::function>(std::move(n->parameters), std::move(n->body), env);

    } else if (auto n{dynamic_cast<ast::call_expression*>(&node)}) {
        auto fn = eval(*n->fn, env);
        if (is_error(fn.get())) {
            return fn;
        }

        auto args = eval_expressions(n->arguments, env);
        if (args.size() == 1 && is_error(args[0].get())) {
            return std::move(args[0]);
        }

        return apply_function(*fn, std::move(args));
    }

    return nullptr;
}

}

}
