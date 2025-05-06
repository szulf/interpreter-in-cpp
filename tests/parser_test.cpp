#include <gtest/gtest.h>

#include "../src/ast.h"
#include "../src/lexer.h"
#include "../src/parser.h"

#include <print>
#include <stdexcept>

static auto check_parser_errors(const interp::parser::parser& p) -> void {
    if (p.errors.empty()) {
        return;
    }

    std::println(stderr, "parser had {} errors", p.errors.size());
    for (const auto& err : p.errors) {
        std::println(stderr, "parser error: {}", err);
    }

    throw std::runtime_error{"parser errors"};
}

static auto test_integer_literal(const interp::ast::expression& il, interp::i64 value) -> void {
    auto& integ{dynamic_cast<const interp::ast::integer_literal&>(il)};

    if (integ.value != value) {
        throw std::runtime_error{std::format("integ.value should be {} is {}.", value, integ.value)};
    }

    if (integ.token_literal() != std::to_string(value)) {
        throw std::runtime_error{std::format("integ.token_literal should be {} is {}.", value, integ.token_literal())};
    }
}

TEST(parser, let_statments) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    let x = 5;
    let y = 10;
    let foobar = 838383;
    )"};

    lexer::lexer l{input};
    parser::parser p{l};

    auto program = p.parse_program();
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 3);

    struct identifier_test {
        std::string expected_identifer{};
    };

    std::array tests{
        identifier_test{"x"},
        identifier_test{"y"},
        identifier_test{"foobar"},
    };

    for (u32 i = 0; const auto& test : tests) {
        auto& stmt = program.statements[i];

        ASSERT_STREQ(stmt->token_literal().c_str(), "let");

        auto& let_stmt = dynamic_cast<const ast::let_statement&>(*stmt);

        ASSERT_STREQ(let_stmt.name.value.c_str(), test.expected_identifer.c_str());
        ASSERT_STREQ(let_stmt.name.token_literal().c_str(), test.expected_identifer.c_str());

        i++;
    }
}

TEST(parser, return_statement) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    return 5;
    return 10;
    return 993322;
    )"};

    lexer::lexer l{input};
    parser::parser p{l};

    auto program = p.parse_program();
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 3);

    for (const auto& stmt : program.statements) {
        auto& _{dynamic_cast<ast::return_statement&>(*stmt)};

        ASSERT_STREQ(stmt->token_literal().c_str(), "return");
    }
}

TEST(parser, ident_expression) {
    using namespace interp;

    static constexpr std::string_view input{"foobar;"};

    lexer::lexer l{input};
    parser::parser p{l};

    auto program = p.parse_program();
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);

    auto& stmt = dynamic_cast<ast::expression_statement&>(*program.statements[0]);
    auto& ident = dynamic_cast<ast::identifier&>(*stmt.expression);

    ASSERT_STREQ(ident.value.c_str(), "foobar");
    ASSERT_STREQ(ident.token_literal().c_str(), "foobar");
}

TEST(parser, integer_expression) {
    using namespace interp;

    static constexpr std::string_view input{"5;"};

    lexer::lexer l{input};
    parser::parser p{l};

    auto program = p.parse_program();
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);

    auto& stmt = dynamic_cast<ast::expression_statement&>(*program.statements[0]);
    auto& int_lit = dynamic_cast<ast::integer_literal&>(*stmt.expression);

    ASSERT_EQ(int_lit.value, 5);
    ASSERT_STREQ(int_lit.token_literal().c_str(), "5");
}

TEST(parser, prefix_expressions) {
    using namespace interp;

    struct prefix_test {
        std::string input{};
        std::string oper{};
        i64 int_value{};
    };

    std::array prefix_tests{
        prefix_test{ "!5;", "!",  5},
        prefix_test{"-15;", "-", 15},
    };

    for (const auto& test : prefix_tests) {
        auto l{lexer::lexer{test.input}};
        auto p{parser::parser{l}};

        auto program{p.parse_program()};
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);

        auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
        auto& exp{dynamic_cast<ast::prefix_expression&>(*stmt.expression)};

        ASSERT_STREQ(exp.oper.c_str(), test.oper.c_str());
        test_integer_literal(*exp.right, test.int_value);
    }
}
