#include "eval.h"
#include "ast.h"
#include "object.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace interp {

namespace eval {

static auto len_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    if (dynamic_cast<object::string*>(args[0].get())) {
        auto& str{dynamic_cast<object::string&>(*args[0])};
        return std::make_unique<object::integer>(str.value.size());
    } else if (dynamic_cast<object::array*>(args[0].get())) {
        auto& arr{dynamic_cast<object::array&>(*args[0])};
        return std::make_unique<object::integer>(arr.elements.size());
    }

    return std::make_unique<object::error>(
        std::format("argument to 'len' not supported, got: {}", object::get_object_type_string(args[0]->type()))
    );
}

static auto first_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    if (dynamic_cast<object::array*>(args[0].get())) {
        auto& arr{dynamic_cast<object::array&>(*args[0])};
        if (arr.elements.size() < 1) {
            return std::make_unique<object::null>();
        }
        return arr.elements.front()->clone();
    }

    return std::make_unique<object::error>(
        std::format("argument to 'first' must be Array, got {}", object::get_object_type_string(args[0]->type()))
    );
}

static auto last_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    if (dynamic_cast<object::array*>(args[0].get())) {
        auto& arr{dynamic_cast<object::array&>(*args[0])};
        if (arr.elements.size() < 1) {
            return std::make_unique<object::null>();
        }
        return arr.elements.back()->clone();
    }

    return std::make_unique<object::error>(
        std::format("argument to 'last' must be Array, got {}", object::get_object_type_string(args[0]->type()))
    );
}

static auto rest_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    if (dynamic_cast<object::array*>(args[0].get())) {
        auto& arr{dynamic_cast<object::array&>(*args[0])};
        if (arr.elements.size() < 1) {
            return std::make_unique<object::null>();
        }

        auto clone = arr.clone();
        auto& arr_clone{dynamic_cast<object::array&>(*clone)};
        arr_clone.elements.erase(arr_clone.elements.begin());

        return clone;
    }

    return std::make_unique<object::error>(
        std::format("argument to 'rest' must be Array, got {}", object::get_object_type_string(args[0]->type()))
    );
}

static auto push_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 2) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 2", args.size()));
    }

    if (dynamic_cast<object::array*>(args[0].get())) {
        auto& arr{dynamic_cast<object::array&>(*args[0])};

        auto clone = arr.clone();
        auto& arr_clone{dynamic_cast<object::array&>(*clone)};
        arr_clone.elements.emplace_back(std::move(args[1]));

        return clone;
    }

    return std::make_unique<object::error>(
        std::format("argument to 'push' must be Array, got {}", object::get_object_type_string(args[0]->type()))
    );
}

static auto puts_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() <= 0) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. needs at least one"));
    }

    for (const auto& arg : args) {
        switch (arg->type()) {
        case object::object_type::String: {
            auto& str{dynamic_cast<object::string&>(*arg)};
            std::println("{}", str.value);
        } break;

        default: {
            std::println("{}", arg->to_string());
        } break;
        }
    }

    return std::make_unique<object::null>();
}

static auto rand_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 2) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 2", args.size()));
    }

    if (args[0]->type() != object::object_type::Integer || args[1]->type() != object::object_type::Integer) {
        return std::make_unique<object::error>(std::format(
            "all arguments to 'rand()' have to be Integers, got: {}, {}",
            object::get_object_type_string(args[0]->type()),
            object::get_object_type_string(args[1]->type())
        ));
    }

    auto& int1{dynamic_cast<object::integer&>(*args[0]).value};
    auto& int2{dynamic_cast<object::integer&>(*args[1]).value};

    i64 min{};
    i64 max{};
    if (int1 >= int2) {
        min = int2;
        max = int1;
    } else {
        min = int1;
        max = int2;
    }

    std::random_device rd{};
    std::mt19937_64 generator{rd()};
    std::uniform_int_distribution<i64> dist(min, max);

    return std::make_unique<object::integer>(dist(generator));
}

static auto gets_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 0) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 0", args.size()));
    }

    std::string line{};
    std::getline(std::cin, line);

    return std::make_unique<object::string>(line);
}

static auto to_string_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    return std::make_unique<object::string>(args[0]->to_string());
}

static auto parse_int_builtin(std::vector<std::unique_ptr<object::object>> args) -> std::unique_ptr<object::object> {
    if (args.size() != 1) {
        return std::make_unique<object::error>(std::format("wrong number of arguments. got: {}, want: 1", args.size()));
    }

    if (args[0]->type() != object::object_type::String) {
        return std::make_unique<object::error>(std::format(
            "argument to 'parse_int()' has to be String, got {}",
            object::get_object_type_string(args[0]->type())
        ));
    }

    auto& str{dynamic_cast<object::string&>(*args[0]).value};
    try {
        return std::make_unique<object::integer>(std::stol(str));
    } catch (const std::exception&) {
        return std::make_unique<object::error>(std::format("invalid argument to function 'parse_int()', got {}", str));
    }
}

static auto builtins = std::unordered_map<std::string, object::builtin>{
    {"len",       {len_builtin}      },
    {"first",     {first_builtin}    },
    {"last",      {last_builtin}     },
    {"rest",      {rest_builtin}     },
    {"push",      {push_builtin}     },
    {"puts",      {puts_builtin}     },
    {"rand",      {rand_builtin}     },
    {"gets",      {gets_builtin}     },
    {"to_string", {to_string_builtin}},
    {"parse_int", {parse_int_builtin}},
};

static auto is_error(object::object* obj) -> bool {
    if (obj) {
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
            return result;
        } break;

        case object::object_type::BreakValue: {
            return std::make_unique<object::error>("break statement is illegal in current context");
        } break;

        case object::object_type::ContinueValue: {
            return std::make_unique<object::error>("continue statement is illegal in current context");
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
            case object::object_type::ContinueValue:
            case object::object_type::BreakValue:
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
        auto left_val{dynamic_cast<const object::boolean&>(left).value};
        auto right_val{dynamic_cast<const object::boolean&>(right).value};

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
    } else if (left.type() == object::object_type::String && right.type() == object::object_type::String) {
        auto& left_val{dynamic_cast<const object::string&>(left).value};
        auto& right_val{dynamic_cast<const object::string&>(right).value};

        if (oper == "+") {
            return std::make_unique<object::string>(left_val + right_val);
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

static auto apply_function(std::unique_ptr<object::object>& function, std::vector<std::unique_ptr<object::object>> args)
    -> std::unique_ptr<object::object> {
    if (dynamic_cast<object::function*>(function.get())) {
        auto& fn{dynamic_cast<object::function&>(*function)};

        auto env{std::make_unique<object::environment>(&fn.env_outer)};
        for (const auto& [param, arg] : std::ranges::zip_view(fn.parameters, args)) {
            env->set(dynamic_cast<const ast::identifier&>(*param).value, std::move(arg));
        }

        auto evaluated{eval(*fn.body, *env)};

        if (dynamic_cast<object::function*>(evaluated.get())) {
            fn.env_outer.envs_inner.push_back(std::move(env));
        }

        if (evaluated && evaluated->type() == object::object_type::BreakValue) {
            return std::make_unique<object::error>("break statement is illegal in current context");
        }

        if (evaluated && evaluated->type() == object::object_type::ContinueValue) {
            return std::make_unique<object::error>("continue statement is illegal in current context");
        }

        if (auto val{dynamic_cast<object::return_value*>(evaluated.get())}) {
            if (dynamic_cast<object::function*>(val->value.get())) {
                fn.env_outer.envs_inner.push_back(std::move(env));
            }

            return std::move(val->value);
        }

        return evaluated;
    } else if (dynamic_cast<object::builtin*>(function.get())) {
        auto& fn{dynamic_cast<object::builtin&>(*function)};
        return fn.fn(std::move(args));
    }

    return std::make_unique<object::error>(
        std::format("not a function: {}", object::get_object_type_string(function->type()))
    );
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
        if (is_error(val.get()) || !val) {
            return val;
        }

        if (val->type() == object::object_type::ReturnValue) {
            auto& ret{dynamic_cast<object::return_value&>(*val)};
            env.set(n->name.value, std::move(ret.value));
            return nullptr;
        }

        if (val->type() == object::object_type::BreakValue) {
            return std::make_unique<object::error>("break statement is illegal in current context");
        }

        if (val->type() == object::object_type::ContinueValue) {
            return std::make_unique<object::error>("continue statement is illegal in current context");
        }

        env.set(n->name.value, std::move(val));

    } else if (auto n{dynamic_cast<ast::identifier*>(&node)}) {
        auto* val{env.get(n->value)};
        if (val && *val) {
            return (*val)->clone();
        }

        if (builtins.contains(n->value)) {
            return builtins[n->value].clone();
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

        return apply_function(fn, std::move(args));

    } else if (auto n{dynamic_cast<ast::string_literal*>(&node)}) {
        return std::make_unique<object::string>(n->value);

    } else if (auto n{dynamic_cast<ast::array_literal*>(&node)}) {
        auto arr{std::make_unique<object::array>()};
        arr->elements = eval_expressions(n->elements, env);

        if (arr->elements.size() == 1 && is_error(arr->elements[0].get())) {
            return std::move(arr->elements[0]);
        }

        return arr;
    } else if (auto n{dynamic_cast<ast::index_expression*>(&node)}) {
        auto left{eval(*n->left, env)};
        if (is_error(left.get())) {
            return left;
        }

        auto index{eval(*n->index, env)};
        if (is_error(index.get())) {
            return index;
        }

        if (left->type() == object::object_type::Array && index->type() == object::object_type::Integer) {
            auto& arr{dynamic_cast<object::array&>(*left)};
            auto& idx{dynamic_cast<object::integer&>(*index).value};

            if (idx >= static_cast<i64>(arr.elements.size()) || idx < 0) {
                return std::make_unique<object::null>();
            }

            return arr.elements[static_cast<usize>(idx)]->clone();
        } else if (left->type() == object::object_type::Hash && dynamic_cast<object::hashable*>(index.get())) {
            auto& hash{dynamic_cast<object::hash&>(*left)};
            auto key{dynamic_cast<object::hashable&>(*index).get_hash_key()};

            if (!hash.pairs.contains(key)) {
                return std::make_unique<object::null>();
            }

            return hash.pairs[key].second->clone();

        } else {
            switch (left->type()) {
            case interp::object::object_type::Hash: {
                return std::make_unique<object::error>(
                    std::format("unusable as hash key: {}", object::get_object_type_string(index->type()))
                );
            } break;

            default: {
                return std::make_unique<object::error>(
                    std::format("index not supported: {}", object::get_object_type_string(left->type()))
                );
            } break;
            }
        }

    } else if (auto n{dynamic_cast<ast::hash_literal*>(&node)}) {
        auto hash{std::make_unique<object::hash>()};

        for (const auto& [key, val] : n->pairs) {
            auto left{eval(*key, env)};
            if (is_error(left.get())) {
                return left;
            }

            auto right{eval(*val, env)};
            if (is_error(right.get())) {
                return right;
            }

            if (auto h{dynamic_cast<object::hashable*>(left.get())}) {
                hash->pairs[h->get_hash_key()] = std::make_pair(std::move(left), std::move(right));
            } else {
                return std::make_unique<object::error>(
                    std::format("unusable as hash key: {}", object::get_object_type_string(left->type()))
                );
            }
        }

        return hash;

    } else if (auto n{dynamic_cast<ast::assign_expression*>(&node)}) {
        auto& ident{dynamic_cast<ast::identifier&>(*n->name)};
        if (!env.contains(ident.value)) {
            return std::make_unique<object::error>(std::format("variable {} does not exist yet", ident.value));
        }

        auto evaluated{eval(*n->value, env)};
        env.update(ident.value, evaluated->clone());

        return evaluated;
    } else if (auto n{dynamic_cast<ast::while_statement*>(&node)}) {
        auto condition{eval(*n->condition, env)};
        if (is_error(condition.get())) {
            return condition;
        }

        while (is_truthy(*condition)) {
            object::environment env_inner{&env};
            auto evaluated{eval(*n->body, env_inner)};
            if (is_error(evaluated.get()) || evaluated->type() == object::object_type::ReturnValue) {
                return evaluated;
            }

            if (evaluated->type() == object::object_type::BreakValue) {
                break;
            }

            condition = eval(*n->condition, env);
            if (is_error(condition.get())) {
                return condition;
            }
        }

    } else if (auto{dynamic_cast<ast::break_statement*>(&node)}) {
        return std::make_unique<object::break_value>();

    } else if (auto{dynamic_cast<ast::continue_statement*>(&node)}) {
        return std::make_unique<object::continue_value>();
    }

    return nullptr;
}
}
}
