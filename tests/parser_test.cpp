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

static auto test_identifier(const interp::ast::expression& expr, std::string_view value) -> void {
    auto& ident{dynamic_cast<const interp::ast::identifier&>(expr)};

    if (ident.value != value) {
        throw std::runtime_error{std::format("ident.value should be {} is {}.", value, ident.value)};
    }

    if (ident.token_literal() != value) {
        throw std::runtime_error{std::format("ident.token_literal() should be {} is {}.", value, ident.token_literal())};
    }
}

static auto test_boolean_literal(const interp::ast::expression& expr, bool value) {
    auto& bool_expr{dynamic_cast<const interp::ast::boolean_expression&>(expr)};

    if (bool_expr.value != value) {
        throw std::runtime_error{std::format("bool_expr.value should be {} is {}.", value, bool_expr.value)};
    }

    if (bool_expr.token_literal() != std::format("{}", value)) {
        throw std::runtime_error{std::format("bool_expr.token_literal() should be {} is {}.", value, bool_expr.token_literal())};
    }
}

template <typename T>
static auto test_literal_expression(const interp::ast::expression& expr, const T& expected) -> void {
    if constexpr (std::is_same_v<T, interp::i64>) {
        test_integer_literal(expr, expected);
    } else if constexpr (std::convertible_to<T, std::string>) {
        test_identifier(expr, expected);
    } else if constexpr (std::is_same_v<T, bool>) {
        test_boolean_literal(expr, expected);
    } else {
        throw std::runtime_error{"Type of literal expression not handled."};
    }
}

template <typename T1, typename T2>
static auto test_infix_expression(const interp::ast::expression& expr, const T1& left, std::string_view oper, const T2& right) -> void {
    auto& in_expr{dynamic_cast<const interp::ast::infix_expression&>(expr)};

    test_literal_expression(*in_expr.left, left);

    if (in_expr.oper != oper) {
        throw std::runtime_error{std::format("expr.oper should be {} is {}.", oper, in_expr.oper)};
    }

    test_literal_expression(*in_expr.right, right);
}

static auto test_let_statement(const interp::ast::statement& stmt, std::string_view name) {
    if (stmt.token_literal() != "let") {
        throw std::runtime_error{std::format("stmt.token_literal() should be {} is {}.", name, stmt.token_literal())};
    }

    auto& let_stmt{dynamic_cast<const interp::ast::let_statement&>(stmt)};

    if (let_stmt.name.value != name) {
        throw std::runtime_error{std::format("let_stmt.name.value should be {} is {}.", name, let_stmt.name.value)};
    }

    if (let_stmt.name.token_literal() != name) {
        throw std::runtime_error{std::format("let_stmt.name.token_literal() should be {} is {}.", name, let_stmt.name.token_literal())};
    }
}

TEST(parser, let_statments) {
    using namespace interp;

    struct let_test {
        std::string_view input{};
        std::string_view expected_identifer{};
        std::variant<i64, bool, std::string> expected_value{};
    };

    static constexpr std::array tests{
        let_test{"let x = 5;",      "x",      5   },
        let_test{"let y = true;",   "y",      true},
        let_test{"let foobar = y;", "foobar", "y" },
    };

    for (const auto& test : tests) {
        lexer::lexer l{test.input};
        parser::parser p{l};
        auto program = p.parse_program();
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);
        auto& stmt{program.statements[0]};

        test_let_statement(*stmt, test.expected_identifer);
        auto& let_stmt{dynamic_cast<interp::ast::let_statement&>(*stmt)};

        std::visit(
            [&](const auto& val) {
                test_literal_expression(*let_stmt.value, val);
            },
            test.expected_value
        );
    }
}

TEST(parser, return_statement) {
    using namespace interp;

    struct return_test {
        std::string_view input{};
        std::variant<i64, bool, std::string> expected_value{};
    };

    static constexpr std::array tests{
        return_test{"return 5;",      5       },
        return_test{"return true;",   true    },
        return_test{"return foobar;", "foobar"},
    };

    for (const auto& test : tests) {
        lexer::lexer l{test.input};
        parser::parser p{l};
        auto program = p.parse_program();
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);
        auto& stmt{program.statements[0]};
        auto& return_stmt{dynamic_cast<ast::return_statement&>(*stmt)};

        ASSERT_EQ(return_stmt.token_literal(), "return");

        std::visit(
            [&](const auto& val) {
                test_literal_expression(*return_stmt.value, val);
            },
            test.expected_value
        );
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
    auto& ident = dynamic_cast<ast::identifier&>(*stmt.expr);

    ASSERT_EQ(ident.value, "foobar");
    ASSERT_EQ(ident.token_literal(), "foobar");
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
    auto& int_lit = dynamic_cast<ast::integer_literal&>(*stmt.expr);

    ASSERT_EQ(int_lit.value, 5);
    ASSERT_EQ(int_lit.token_literal(), "5");
}

TEST(parser, prefix_expressions) {
    using namespace interp;

    struct prefix_test {
        std::string_view input{};
        std::string_view oper{};
        std::variant<i64, bool, std::string> expected_value{};
    };

    static constexpr std::array prefix_tests{
        prefix_test{"!5;",      "!", 5       },
        prefix_test{"-15;",     "-", 15      },
        prefix_test{"!foobar;", "!", "foobar"},
        prefix_test{"-foobar;", "-", "foobar"},
        prefix_test{"!true;",   "!", true    },
        prefix_test{"!false;",  "!", false   },
    };

    for (const auto& test : prefix_tests) {
        auto l{lexer::lexer{test.input}};
        auto p{parser::parser{l}};
        auto program{p.parse_program()};
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);

        auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
        auto& expr{dynamic_cast<ast::prefix_expression&>(*stmt.expr)};

        ASSERT_EQ(expr.oper, test.oper);
        std::visit(
            [&](const auto& val) {
                test_literal_expression(*expr.right, val);
            },
            test.expected_value
        );
    }
}

TEST(parser, infix_expression) {
    using namespace interp;

    struct infix_test {
        std::string_view input{};
        std::variant<i64, bool, std::string> left_val{};
        std::string_view oper{};
        std::variant<i64, bool, std::string> right_val{};
    };

    static constexpr std::array tests{
        infix_test{"5 + 5;",            5,        "+",  5       },
        infix_test{"5 - 5;",            5,        "-",  5       },
        infix_test{"5 * 5;",            5,        "*",  5       },
        infix_test{"5 / 5;",            5,        "/",  5       },
        infix_test{"5 > 5;",            5,        ">",  5       },
        infix_test{"5 < 5;",            5,        "<",  5       },
        infix_test{"5 == 5;",           5,        "==", 5       },
        infix_test{"5 != 5;",           5,        "!=", 5       },
        infix_test{"foobar + barfoo;",  "foobar", "+",  "barfoo"},
        infix_test{"foobar - barfoo;",  "foobar", "-",  "barfoo"},
        infix_test{"foobar * barfoo;",  "foobar", "*",  "barfoo"},
        infix_test{"foobar / barfoo;",  "foobar", "/",  "barfoo"},
        infix_test{"foobar > barfoo;",  "foobar", ">",  "barfoo"},
        infix_test{"foobar < barfoo;",  "foobar", "<",  "barfoo"},
        infix_test{"foobar == barfoo;", "foobar", "==", "barfoo"},
        infix_test{"foobar != barfoo;", "foobar", "!=", "barfoo"},
        infix_test{"true == true",      true,     "==", true    },
        infix_test{"true != false",     true,     "!=", false   },
        infix_test{"false == false",    false,    "==", false   },
    };

    for (const auto& test : tests) {
        lexer::lexer l{test.input};
        parser::parser p{l};
        auto program{p.parse_program()};
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);
        auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};

        std::visit(
            [&](const auto& left, const auto& right) {
                test_infix_expression(*stmt.expr, left, test.oper, right);
            },
            test.left_val,
            test.right_val
        );
    }
}

TEST(parser, operator_precedence) {
    using namespace interp;

    struct precedence_test {
        std::string_view input{};
        std::string_view expected{};
    };

    static constexpr std::array tests{
        precedence_test{"-a * b",                     "((-a) * b)"                            },
        precedence_test{"!-a",                        "(!(-a))"                               },
        precedence_test{"a + b + c",                  "((a + b) + c)"                         },
        precedence_test{"a + b - c",                  "((a + b) - c)"                         },
        precedence_test{"a * b * c",                  "((a * b) * c)"                         },
        precedence_test{"a * b / c",                  "((a * b) / c)"                         },
        precedence_test{"a + b / c",                  "(a + (b / c))"                         },
        precedence_test{"a + b * c + d / e - f",      "(((a + (b * c)) + (d / e)) - f)"       },
        precedence_test{"3 + 4; -5 * 5",              "(3 + 4)((-5) * 5)"                     },
        precedence_test{"5 > 4 == 3 < 4",             "((5 > 4) == (3 < 4))"                  },
        precedence_test{"5 < 4 != 3 > 4",             "((5 < 4) != (3 > 4))"                  },
        precedence_test{"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        precedence_test{"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        precedence_test{"true",                       "true"                                  },
        precedence_test{"false",                      "false"                                 },
        precedence_test{"3 > 5 == false",             "((3 > 5) == false)"                    },
        precedence_test{"3 < 5 == true",              "((3 < 5) == true)"                     },
        precedence_test{"1 + (2 + 3) + 4",            "((1 + (2 + 3)) + 4)"                   },
        precedence_test{"(5 + 5) * 2",                "((5 + 5) * 2)"                         },
        precedence_test{"2 / (5 + 5)",                "(2 / (5 + 5))"                         },
        precedence_test{"(5 + 5) * 2 * (5 + 5)",      "(((5 + 5) * 2) * (5 + 5))"             },
        precedence_test{"-(5 + 5)",                   "(-(5 + 5))"                            },
        precedence_test{"!(true == true)",            "(!(true == true))"                     },
        // precedence_test{"a + add(b * c) + d",         "((a + add((b * c))) + d)"              },
        // precedence_test{"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        // precedence_test{"add(a + b + c * d / f + g)",                "add((((a + b) + ((c * d) / f)) + g))"           },
    };

    for (const auto& test : tests) {
        lexer::lexer l{test.input};
        parser::parser p{l};
        auto program{p.parse_program()};
        check_parser_errors(p);

        ASSERT_EQ(program.to_string(), test.expected);
    }
}

TEST(parser, if_expression) {
    using namespace interp;

    static constexpr std::string_view input{"if (x < y) { x }"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& expr{dynamic_cast<ast::if_expression&>(*stmt.expr)};

    test_infix_expression(*expr.condition, "x", "<", "y");
    ASSERT_EQ(expr.consequence->statements.size(), 1);

    auto& consequence{dynamic_cast<ast::expression_statement&>(*expr.consequence->statements[0])};
    test_identifier(*consequence.expr, "x");

    ASSERT_EQ(expr.alternative, nullptr);
}

TEST(parser, if_else_expression) {
    using namespace interp;

    static constexpr std::string_view input{"if (x < y) { x } else { y }"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};

    auto& expr{dynamic_cast<ast::if_expression&>(*stmt.expr)};

    test_infix_expression(*expr.condition, "x", "<", "y");

    ASSERT_EQ(expr.consequence->statements.size(), 1);
    auto& consequence{dynamic_cast<ast::expression_statement&>(*expr.consequence->statements[0])};
    test_identifier(*consequence.expr, "x");

    ASSERT_EQ(expr.alternative->statements.size(), 1);
    auto& alternative{dynamic_cast<ast::expression_statement&>(*expr.alternative->statements[0])};
    test_identifier(*alternative.expr, "y");
}
