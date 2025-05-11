#include "eval.h"
#include "ast.h"
#include "object.h"

namespace interp {

namespace eval {

static auto eval_statements(std::vector<std::unique_ptr<ast::statement>>& stmts) -> std::unique_ptr<object::object> {
    std::unique_ptr<object::object> result{nullptr};

    for (const auto& stmt : stmts) {
        result = eval(*stmt);
    }

    return result;
}

auto eval(ast::node& node) -> std::unique_ptr<object::object> {
    if (auto n{dynamic_cast<ast::program*>(&node)}) {
        return eval_statements(n->statements);
    } else if (auto n{dynamic_cast<ast::expression_statement*>(&node)}) {
        return eval(*n->expr);
    } else if (auto n{dynamic_cast<ast::integer_literal*>(&node)}) {
        return std::make_unique<object::integer>(object::integer{n->value});
    }

    return nullptr;
}
}

}
