#include "ast.h"
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

auto identifier::token_literal() const -> std::string {
    return token.literal;
}

auto identifier::to_string() const -> std::string {
    return value;
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

auto expression_statement::token_literal() const -> std::string {
    return token.literal;
}

auto expression_statement::to_string() const -> std::string {
    if (expr != nullptr) {
        return expr->to_string();
    }

    return {};
}

auto integer_literal::token_literal() const -> std::string {
    return token.literal;
}

auto integer_literal::to_string() const -> std::string {
    return token.literal;
}

auto prefix_expression::token_literal() const -> std::string {
    return token.literal;
}

auto prefix_expression::to_string() const -> std::string {
    return std::format("({}{})", oper, right->to_string());
}

auto infix_expression::token_literal() const -> std::string {
    return token.literal;
}

auto infix_expression::to_string() const -> std::string {
    return std::format("({} {} {})", left->to_string(), oper, right->to_string());
}

auto boolean_expression::token_literal() const -> std::string {
    return token.literal;
}

auto boolean_expression::to_string() const -> std::string {
    return token.literal;
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

}

}
