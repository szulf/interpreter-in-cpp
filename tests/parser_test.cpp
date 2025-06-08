#include <gtest/gtest.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

#include <print>
#include <ranges>
#include <stdexcept>
#include <unordered_map>

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
        throw std::runtime_error{std::format("ident.token_literal() should be {} is {}.", value, ident.token_literal())
        };
    }
}

static auto test_boolean_literal(const interp::ast::expression& expr, bool value) {
    auto& bool_expr{dynamic_cast<const interp::ast::boolean_expression&>(expr)};

    if (bool_expr.value != value) {
        throw std::runtime_error{std::format("bool_expr.value should be {} is {}.", value, bool_expr.value)};
    }

    if (bool_expr.token_literal() != std::format("{}", value)) {
        throw std::runtime_error{
            std::format("bool_expr.token_literal() should be {} is {}.", value, bool_expr.token_literal())
        };
    }
}

template <typename T>
static auto test_literal_expression(const interp::ast::expression& expr, const T& expected) -> void {
    if constexpr (std::is_same_v<T, interp::i64> || std::is_same_v<T, interp::i32>) {
        test_integer_literal(expr, expected);
    } else if constexpr (std::is_convertible_v<T, std::string> || std::is_same_v<T, std::string_view>) {
        test_identifier(expr, expected);
    } else if constexpr (std::is_same_v<T, bool>) {
        test_boolean_literal(expr, expected);
    } else {
        throw std::runtime_error{"Type of literal expression not handled."};
    }
}

template <typename T1, typename T2>
static auto
test_infix_expression(const interp::ast::expression& expr, const T1& left, std::string_view oper, const T2& right)
    -> void {
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
        throw std::runtime_error{
            std::format("let_stmt.name.token_literal() should be {} is {}.", name, let_stmt.name.token_literal())
        };
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
        precedence_test{"-a * b",                                    "((-a) * b)"                                     },
        precedence_test{"!-a",                                       "(!(-a))"                                        },
        precedence_test{"a + b + c",                                 "((a + b) + c)"                                  },
        precedence_test{"a + b - c",                                 "((a + b) - c)"                                  },
        precedence_test{"a * b * c",                                 "((a * b) * c)"                                  },
        precedence_test{"a * b / c",                                 "((a * b) / c)"                                  },
        precedence_test{"a + b / c",                                 "(a + (b / c))"                                  },
        precedence_test{"a + b * c + d / e - f",                     "(((a + (b * c)) + (d / e)) - f)"                },
        precedence_test{"3 + 4; -5 * 5",                             "(3 + 4)((-5) * 5)"                              },
        precedence_test{"5 > 4 == 3 < 4",                            "((5 > 4) == (3 < 4))"                           },
        precedence_test{"5 < 4 != 3 > 4",                            "((5 < 4) != (3 > 4))"                           },
        precedence_test{"3 + 4 * 5 == 3 * 1 + 4 * 5",                "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"         },
        precedence_test{"3 + 4 * 5 == 3 * 1 + 4 * 5",                "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"         },
        precedence_test{"true",                                      "true"                                           },
        precedence_test{"false",                                     "false"                                          },
        precedence_test{"3 > 5 == false",                            "((3 > 5) == false)"                             },
        precedence_test{"3 < 5 == true",                             "((3 < 5) == true)"                              },
        precedence_test{"1 + (2 + 3) + 4",                           "((1 + (2 + 3)) + 4)"                            },
        precedence_test{"(5 + 5) * 2",                               "((5 + 5) * 2)"                                  },
        precedence_test{"2 / (5 + 5)",                               "(2 / (5 + 5))"                                  },
        precedence_test{"(5 + 5) * 2 * (5 + 5)",                     "(((5 + 5) * 2) * (5 + 5))"                      },
        precedence_test{"-(5 + 5)",                                  "(-(5 + 5))"                                     },
        precedence_test{"!(true == true)",                           "(!(true == true))"                              },
        precedence_test{"a + add(b * c) + d",                        "((a + add((b * c))) + d)"                       },
        precedence_test{"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        precedence_test{"add(a + b + c * d / f + g)",                "add((((a + b) + ((c * d) / f)) + g))"           },
        precedence_test{"a * [1, 2, 3, 4][b * c] * d",               "((a * ([1, 2, 3, 4][(b * c)])) * d)"            },
        precedence_test{"add(a * b[2], b[1], 2 * [1, 2][1])",        "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"   },
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
    auto consq = dynamic_cast<ast::block_statement&>(*expr.consequence);
    ASSERT_EQ(consq.statements.size(), 1);

    auto& consequence{dynamic_cast<ast::expression_statement&>(*consq.statements[0])};
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

    auto consq = dynamic_cast<ast::block_statement&>(*expr.consequence);
    ASSERT_EQ(consq.statements.size(), 1);
    auto& consequence{dynamic_cast<ast::expression_statement&>(*consq.statements[0])};
    test_identifier(*consequence.expr, "x");

    auto alter = dynamic_cast<ast::block_statement&>(*expr.alternative);
    ASSERT_EQ(alter.statements.size(), 1);
    auto& alternative{dynamic_cast<ast::expression_statement&>(*alter.statements[0])};
    test_identifier(*alternative.expr, "y");
}

TEST(parser, fn_literal) {
    using namespace interp;

    static constexpr std::string_view input{"fn(x, y) { x + y }"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};

    auto& fn{dynamic_cast<ast::fn_expression&>(*stmt.expr)};
    ASSERT_EQ(fn.parameters.size(), 2);

    test_literal_expression(*fn.parameters[0], "x");
    test_literal_expression(*fn.parameters[1], "y");

    auto body = dynamic_cast<ast::block_statement&>(*fn.body);
    ASSERT_EQ(body.statements.size(), 1);
    auto& body_stmt{dynamic_cast<ast::expression_statement&>(*body.statements[0])};

    test_infix_expression(*body_stmt.expr, "x", "+", "y");
}

TEST(parser, fn_parameter) {
    using namespace interp;

    struct parameter_test {
        std::string_view input{};
        std::vector<std::string_view> expected_params{};
    };

    const std::array tests{
        parameter_test{"fn() {};",        {}             },
        parameter_test{"fn(x) {};",       {"x"}          },
        parameter_test{"fn(x, y, z) {};", {"x", "y", "z"}},
    };

    for (const auto& test : tests) {
        lexer::lexer l{test.input};
        parser::parser p{l};
        auto program{p.parse_program()};
        check_parser_errors(p);

        ASSERT_EQ(program.statements.size(), 1);
        auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
        auto& fn{dynamic_cast<ast::fn_expression&>(*stmt.expr)};

        ASSERT_EQ(fn.parameters.size(), test.expected_params.size());
        for (u32 i = 0; i < fn.parameters.size(); i++) {
            test_literal_expression(*fn.parameters.at(i), test.expected_params.at(i));
        }
    }
}

TEST(parser, call_expression) {
    using namespace interp;

    static constexpr std::string_view input{"add(1, 2 * 3, 4 + 5);"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& call{dynamic_cast<ast::call_expression&>(*stmt.expr)};

    test_identifier(*call.fn, "add");

    ASSERT_EQ(call.arguments.size(), 3);
    test_literal_expression(*call.arguments[0], 1);
    test_infix_expression(*call.arguments[1], 2, "*", 3);
    test_infix_expression(*call.arguments[2], 4, "+", 5);
}

TEST(parser, string_literals) {
    using namespace interp;

    static constexpr std::string_view input{"\"hello world\";"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& str{dynamic_cast<ast::string_literal&>(*stmt.expr)};
    ASSERT_EQ(str.value, "hello world");
}

TEST(parser, array_literals) {
    using namespace interp;

    static constexpr std::string_view input{"[1, 2 * 2, 3 + 3]"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& arr{dynamic_cast<ast::array_literal&>(*stmt.expr)};
    test_integer_literal(*arr.elements[0], 1);
    test_infix_expression(*arr.elements[1], 2, "*", 2);
    test_infix_expression(*arr.elements[2], 3, "+", 3);
}

TEST(parser, index_expression) {
    using namespace interp;

    static constexpr std::string_view input{"myArray[1 + 1]"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& index{dynamic_cast<ast::index_expression&>(*stmt.expr)};
    test_identifier(*index.left, "myArray");
    test_infix_expression(*index.index, 1, "+", 1);
}

TEST(parser, hash_literals_string) {
    using namespace interp;

    static constexpr std::string_view input{"{\"one\": 1, \"two\": 2, \"three\": 3}"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& hash{dynamic_cast<ast::hash_literal&>(*stmt.expr)};
    ASSERT_EQ(hash.pairs.size(), 3);

    std::unordered_map<std::string, i64> expected{
        {"one",   1},
        {"two",   2},
        {"three", 3},
    };

    for (const auto& [key, val] : hash.pairs) {
        auto& literal{dynamic_cast<ast::string_literal&>(*key)};
        auto& expected_val{expected[literal.to_string()]};
        test_integer_literal(*val, expected_val);
    }
}

TEST(parser, hash_literal_empty) {
    using namespace interp;

    static constexpr std::string_view input{"{}"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& hash{dynamic_cast<ast::hash_literal&>(*stmt.expr)};
    ASSERT_EQ(hash.pairs.size(), 0);
}

TEST(parser, hash_literal_boolean) {
    using namespace interp;

    static constexpr std::string_view input{"{true: 1, false: 2}"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& hash{dynamic_cast<ast::hash_literal&>(*stmt.expr)};
    ASSERT_EQ(hash.pairs.size(), 2);

    std::unordered_map<std::string, i64> expected{
        {"true",  1},
        {"false", 2},
    };

    for (const auto& [key, val] : hash.pairs) {
        auto& literal{dynamic_cast<ast::boolean_expression&>(*key)};
        auto& expected_val{expected[literal.to_string()]};
        test_integer_literal(*val, expected_val);
    }
}

TEST(parser, hash_literal_int) {
    using namespace interp;

    static constexpr std::string_view input{"{1: 1, 2: 2, 3: 3}"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& hash{dynamic_cast<ast::hash_literal&>(*stmt.expr)};
    ASSERT_EQ(hash.pairs.size(), 3);

    std::unordered_map<std::string, i64> expected{
        {"1", 1},
        {"2", 2},
        {"3", 3},
    };

    for (const auto& [key, val] : hash.pairs) {
        auto& literal{dynamic_cast<ast::integer_literal&>(*key)};
        auto& expected_val{expected[literal.to_string()]};
        test_integer_literal(*val, expected_val);
    }
}

TEST(parser, hash_literal_expr) {
    using namespace interp;

    static constexpr std::string_view input{"{\"one\": 0 + 1, \"two\": 10 - 8, \"three\": 15 / 5}"};

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};
    check_parser_errors(p);

    ASSERT_EQ(program.statements.size(), 1);
    auto& stmt{dynamic_cast<ast::expression_statement&>(*program.statements[0])};
    auto& hash{dynamic_cast<ast::hash_literal&>(*stmt.expr)};
    ASSERT_EQ(hash.pairs.size(), 3);

    std::unordered_map<std::string, std::function<void(const ast::expression&)>> expected{
        {"one",
         [](const ast::expression& e) {
             test_infix_expression(e, 0, "+", 1);
         }},
        {"two",
         [](const ast::expression& e) {
             test_infix_expression(e, 10, "-", 8);
         }},
        {"three",
         [](const ast::expression& e) {
             test_infix_expression(e, 15, "/", 5);
         }},
    };

    for (const auto& [key, val] : hash.pairs) {
        auto& literal{dynamic_cast<ast::string_literal&>(*key)};
        auto& test_func{expected[literal.to_string()]};
        test_func(*val);
    }
}
