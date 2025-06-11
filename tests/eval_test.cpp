#include <cstddef>
#include <gtest/gtest.h>

#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "types.h"
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>

static auto test_eval(std::string_view input) -> std::unique_ptr<interp::object::object> {
    using namespace interp;

    auto l{lexer::lexer{input}};
    auto p{parser::parser{l}};
    auto program{p.parse_program()};
    auto env{object::environment{}};

    auto x = eval::eval(program, env);

    return x;
}

static auto test_int_object(const interp::object::object& obj, interp::i64 expected) -> void {
    using namespace interp;

    auto& result{dynamic_cast<const object::integer&>(obj)};

    if (result.value != expected) {
        throw std::runtime_error{std::format("result.value should be {} is {}.", expected, result.value)};
    }
}

static auto test_bool_object(const interp::object::object& obj, bool expected) -> void {
    using namespace interp;

    auto& result{dynamic_cast<const object::boolean&>(obj)};

    if (result.value != expected) {
        throw std::runtime_error{std::format("result.value should be {} is {}.", expected, result.value)};
    }
}

static auto test_null_object(const interp::object::object& obj) -> void {
    using namespace interp;

    [[maybe_unused]] auto& result{dynamic_cast<const object::null&>(obj)};
}

TEST(eval, int_expression) {
    using namespace interp;

    struct int_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        int_test{"5",                               5  },
        int_test{"10",                              10 },
        int_test{"-5",                              -5 },
        int_test{"-10",                             -10},
        int_test{"5 + 5 + 5 + 5 - 10",              10 },
        int_test{"2 * 2 * 2 * 2 * 2",               32 },
        int_test{"-50 + 100 + -50",                 0  },
        int_test{"5 * 2 + 10",                      20 },
        int_test{"5 + 2 * 10",                      25 },
        int_test{"20 + 2 * -10",                    0  },
        int_test{"50 / 2 * 2 + 10",                 60 },
        int_test{"2 * (5 + 10)",                    30 },
        int_test{"3 * 3 * 3 + 10",                  37 },
        int_test{"3 * (3 * 3) + 10",                37 },
        int_test{"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50 },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_int_object(*evaluated, test.expected);
    }
}

TEST(eval, bool_expression) {
    using namespace interp;

    struct bool_test {
        std::string_view input{};
        bool expected{};
    };

    static constexpr std::array tests{
        bool_test{"true",             true },
        bool_test{"false",            false},
        bool_test{"1 < 2",            true },
        bool_test{"1 > 2",            false},
        bool_test{"1 < 1",            false},
        bool_test{"1 > 1",            false},
        bool_test{"1 == 1",           true },
        bool_test{"1 != 1",           false},
        bool_test{"1 == 2",           false},
        bool_test{"1 != 2",           true },
        bool_test{"true == true",     true },
        bool_test{"false == false",   true },
        bool_test{"true == false",    false},
        bool_test{"true != false",    true },
        bool_test{"false != true",    true },
        bool_test{"(1 < 2) == true",  true },
        bool_test{"(1 < 2) == false", false},
        bool_test{"(1 > 2) == true",  false},
        bool_test{"(1 > 2) == false", true },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_bool_object(*evaluated, test.expected);
    }
}

TEST(eval, bang_operator) {
    using namespace interp;

    struct bang_test {
        std::string_view input{};
        bool expected{};
    };

    static constexpr std::array tests{
        bang_test{"!true",   false},
        bang_test{"!false",  true },
        bang_test{"!5",      false},
        bang_test{"!!true",  true },
        bang_test{"!!false", false},
        bang_test{"!!5",     true },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_bool_object(*evaluated, test.expected);
    }
}

TEST(eval, if_else) {
    using namespace interp;

    struct if_test {
        std::string_view input{};
        std::variant<i64, std::nullptr_t> expected{};
    };

    static constexpr std::array tests{
        if_test{"if (true) { 10 }",              10     },
        if_test{"if (false) { 10 }",             nullptr},
        if_test{"if (1) { 10 }",                 10     },
        if_test{"if (1 < 2) { 10 }",             10     },
        if_test{"if (1 > 2) { 10 }",             nullptr},
        if_test{"if (1 > 2) { 10 } else { 20 }", 20     },
        if_test{"if (1 < 2) { 10 } else { 20 }", 10     },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        std::visit(
            [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, i64>) {
                    test_int_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    test_null_object(*evaluated);
                }
            },
            test.expected
        );
    }
}

TEST(eval, return) {
    using namespace interp;

    struct return_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        return_test{"return 10;",                 10},
        return_test{"return 10; 9;",              10},
        return_test{"return 2 * 5; 9;",           10},
        return_test{"9; return 2 * 5; 9;",        10},
        return_test{"if (10 > 1) { return 10; }", 10},
        return_test{
                    R"(
        if (10 > 1) {
          if (10 > 1) {
            return 10;
          }

          return 1;
        })",                          10
        },
        return_test{
                    R"(
        let f = fn(x) {
          return x;
          x + 10;
        };
        f(10);)",                          10
        },
        return_test{
                    R"(
        let f = fn(x) {
           let result = x + 10;
           return result;
           return 10;
        };
        f(10);)",                          20
        }
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_int_object(*evaluated, test.expected);
    }
}

TEST(eval, error) {
    using namespace interp;

    struct error_test {
        std::string_view input{};
        std::string_view expected_message{};
    };

    static constexpr std::array tests{
        error_test{"5 + true;",                            "type mismatch: Integer + Boolean"   },
        error_test{"5 + true; 5;",                         "type mismatch: Integer + Boolean"   },
        error_test{"-true",                                "unknown operator: -Boolean"         },
        error_test{"true + false;",                        "unknown operator: Boolean + Boolean"},
        error_test{"5; true + false; 5",                   "unknown operator: Boolean + Boolean"},
        error_test{"if (10 > 1) { true + false; }",        "unknown operator: Boolean + Boolean"},
        error_test{
                   R"(
        if (10 > 1) {
            if (10 > 1) {
                return true + false;
            }
            return 1;
        })",                                    "unknown operator: Boolean + Boolean"
        },
        error_test{"foobar",                               "identifier not found: foobar"       },
        error_test{"\"Hello\" - \"World\"",                "unknown operator: String - String"  },
        error_test{"{\"name\": \"Monkey\"}[fn(x) { x }];", "unusable as hash key: Function"     }
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        auto err = dynamic_cast<object::error&>(*evaluated);
        ASSERT_EQ(err.message, test.expected_message);
    }
}

TEST(eval, let_statement) {
    using namespace interp;

    struct let_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        let_test{"let a = 5; a;",                               5 },
        let_test{"let a = 5 * 5; a;",                           25},
        let_test{"let a = 5; let b = a; b;",                    5 },
        let_test{"let a = 5; let b = a; let c = a + b + 5; c;", 15},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_int_object(*evaluated, test.expected);
    }
}

TEST(eval, function_object) {
    using namespace interp;

    static constexpr std::string_view input{"fn(x) { x + 2; };"};

    auto evaluated{test_eval(input)};
    auto& fn = dynamic_cast<object::function&>(*evaluated);
    ASSERT_EQ(fn.parameters.size(), 1);
    ASSERT_EQ(fn.parameters[0]->to_string(), "x");

    static constexpr std::string_view expected_body = "(x + 2)";
    ASSERT_EQ(expected_body, fn.body->to_string());
}

TEST(eval, function_application) {
    using namespace interp;

    struct fn_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        fn_test{"let identity = fn(x) { x; }; identity(5);",             5 },
        fn_test{"let identity = fn(x) { return x; }; identity(5);",      5 },
        fn_test{"let double = fn(x) { x * 2; }; double(5);",             10},
        fn_test{"let add = fn(x, y) { x + y; }; add(5, 5);",             10},
        fn_test{"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
        fn_test{"fn(x) { x; }(5)",                                       5 },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_int_object(*evaluated, test.expected);
    }
}

TEST(eval, closures) {
    using namespace interp;

    static constexpr std::string_view input{R"(
let newAdder = fn(x) {
fn(y) { x + y };
};
let addTwo = newAdder(2);
addTwo(2);)"};

    auto evaluated{test_eval(input)};
    test_int_object(*evaluated, 4);
}

TEST(eval, strings) {
    using namespace interp;

    static constexpr std::string_view input{"\"Hello World!\""};

    auto evaluated{test_eval(input)};
    auto& str{dynamic_cast<object::string&>(*evaluated)};
    ASSERT_EQ(str.value, "Hello World!");
}

TEST(eval, string_concat) {
    using namespace interp;

    static constexpr std::string_view input{"\"Hello\" + \" \" + \"World\""};

    auto evaluated{test_eval(input)};
    auto& str{dynamic_cast<object::string&>(*evaluated)};
    ASSERT_EQ(str.value, "Hello World");
}

TEST(eval, builtins) {
    using namespace interp;

    struct builtin_test {
        std::string_view input{};
        std::variant<i64, std::string, std::nullptr_t, std::vector<i64>> expected{};
    };

    std::array tests{
        builtin_test{"len(\"\")",             0                                               },
        builtin_test{"len(\"four\")",         4                                               },
        builtin_test{"len(\"hello world\")",  11                                              },
        builtin_test{"len(1)",                "argument to 'len' not supported, got: Integer" },
        builtin_test{"len(\"one\", \"two\")", "wrong number of arguments. got: 2, want: 1"    },
        builtin_test{"len([1, 2, 3])",        3                                               },
        builtin_test{"len([])",               0                                               },
        builtin_test{"first([1, 2, 3])",      1                                               },
        builtin_test{"first([])",             nullptr                                         },
        builtin_test{"first(1)",              "argument to 'first' must be Array, got Integer"},
        builtin_test{"last([1, 2, 3])",       3                                               },
        builtin_test{"last([])",              nullptr                                         },
        builtin_test{"last(1)",               "argument to 'last' must be Array, got Integer" },
        builtin_test{"rest([1, 2, 3])",       std::vector<i64>{2, 3}                          },
        builtin_test{"rest([])",              nullptr                                         },
        builtin_test{"push([], 1)",           std::vector<i64>{1}                             },
        builtin_test{"push(1, 1)",            "argument to 'push' must be Array, got Integer" },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        std::visit(
            [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, i64>) {
                    test_int_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto& err{dynamic_cast<object::error&>(*evaluated)};
                    ASSERT_EQ(err.message, val);
                } else if constexpr (std::is_same_v<T, std::vector<i64>>) {
                    auto& arr{dynamic_cast<object::array&>(*evaluated)};

                    ASSERT_EQ(arr.elements.size(), val.size());
                    for (const auto& [elem, v] : std::ranges::zip_view(arr.elements, val)) {
                        test_int_object(*elem, v);
                    }
                } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    test_null_object(*evaluated);
                }
            },
            test.expected
        );
    }
}

TEST(eval, array_literal) {
    using namespace interp;

    static constexpr std::string_view input{"[1, 2 * 2, 3 + 3]"};

    auto evaluated{test_eval(input)};
    auto& result{dynamic_cast<object::array&>(*evaluated)};
    ASSERT_EQ(result.elements.size(), 3);

    test_int_object(*result.elements[0], 1);
    test_int_object(*result.elements[1], 4);
    test_int_object(*result.elements[2], 6);
}

TEST(eval, index_expression) {
    using namespace interp;

    struct index_test {
        std::string_view input{};
        std::optional<i64> expected{};
    };

    std::array tests{
        index_test{"[1, 2, 3][0]",                                                   1           },
        index_test{"[1, 2, 3][1]",                                                   2           },
        index_test{"[1, 2, 3][2]",                                                   3           },
        index_test{"let i = 0; [1][i];",                                             1           },
        index_test{"[1, 2, 3][1 + 1];",                                              3           },
        index_test{"let myArray = [1, 2, 3]; myArray[2];",                           3           },
        index_test{"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6           },
        index_test{"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]",        2           },
        index_test{"[1, 2, 3][3]",                                                   std::nullopt},
        index_test{"[1, 2, 3][-1]",                                                  std::nullopt},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        if (test.expected.has_value()) {
            test_int_object(*evaluated, *test.expected);
        } else {
            test_null_object(*evaluated);
        }
    }
}

TEST(eval, hash_literals) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    let two = "two";
    {
        "one": 10 - 9,
        two: 1 + 1,
        "thr" + "ee": 6 / 2,
        4: 4,
        true: 5,
        false: 6
    }
    )"};

    auto evaluated{test_eval(input)};
    auto& hash{dynamic_cast<object::hash&>(*evaluated)};

    std::unordered_map<object::hash_key, i64> expected{
        {object::string{"one"}.get_hash_key(),   1},
        {object::string{"two"}.get_hash_key(),   2},
        {object::string{"three"}.get_hash_key(), 3},
        {object::integer{4}.get_hash_key(),      4},
        {object::boolean{true}.get_hash_key(),   5},
        {object::boolean{false}.get_hash_key(),  6},
    };

    ASSERT_EQ(hash.pairs.size(), expected.size());

    for (const auto& [expected_key, expected_val] : expected) {
        auto& pair{hash.pairs[expected_key]};

        test_int_object(*pair.second, expected_val);
    }
}

TEST(eval, index_hash_expressions) {
    using namespace interp;

    struct hash_test {
        std::string_view input{};
        std::optional<i64> expected{};
    };

    std::array tests{
        hash_test{"{\"foo\" : 5}[\"foo\"]",                5           },
        hash_test{"{\"foo\" : 5}[\"bar\"]",                std::nullopt},
        hash_test{"let key = \"foo\"; {\"foo\" : 5}[key]", 5           },
        hash_test{"{}[\"foo\"]",                           std::nullopt},
        hash_test{"{5 : 5}[5]",                            5           },
        hash_test{"{true : 5}[true]",                      5           },
        hash_test{"{false : 5}[false]",                    5           },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        if (test.expected.has_value()) {
            test_int_object(*evaluated, *test.expected);
        } else {
            test_null_object(*evaluated);
        }
    }
}

TEST(eval, assign_expression) {
    using namespace interp;

    struct assign_test {
        std::string_view input{};
        std::variant<i64, std::string, bool> expected{};
    };

    std::array tests{
        assign_test{"let x = 5; x = 3",        3                              },
        assign_test{"let x = false; x = true", true                           },
        assign_test{"x = 3;",                  "variable x does not exist yet"},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        std::visit(
            [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, i64>) {
                    test_int_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, bool>) {
                    test_bool_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto& error{dynamic_cast<object::error&>(*evaluated)};
                    ASSERT_EQ(error.message, val);
                }
            },
            test.expected
        );
    }
}

TEST(eval, assign_expression_chain) {
    using namespace interp;

    static constexpr std::string_view input{R"(
    let x = 5;
    let z = 3;
    let foo = x;
    let bar = "bar";
    x = z = foo = bar;
    z + foo;
    )"};

    auto evaluated{test_eval(input)};

    auto& str{dynamic_cast<object::string&>(*evaluated)};
    ASSERT_EQ(str.value, "barbar");
}

TEST(eval, while_statement) {
    using namespace interp;

    struct while_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        while_test{
                   R"(
    let x = 0;

    while (x < 5) {
        x = x + 1;
    }

    x)", 5
        },
        while_test{
                   R"(
    let x = 0;

    while (x < 5) {
        x = x + 1;
        return x;
    }

    x)", 1
        },
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        auto& integer{dynamic_cast<object::integer&>(*evaluated)};
        ASSERT_EQ(integer.value, test.expected);
    }
}

TEST(eval, break_statement) {
    using namespace interp;

    struct break_test {
        std::string_view input{};
        std::variant<i64, std::string> expected{};
    };

    std::array tests{
        break_test{
                   R"(
        let x = 0;

        while (x < 5) {
            x = x + 1;

            if (x == 3) {
                break;
            }
        }

        x)",                            3
        },
        break_test{"let x = if (true) { break; }", "break statement is illegal in current context"},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        std::visit(
            [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, i64>) {
                    test_int_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto err{dynamic_cast<object::error&>(*evaluated)};
                    ASSERT_EQ(err.message, val);
                }
            },
            test.expected
        );
    }
}

TEST(eval, continue_statement) {
    using namespace interp;

    struct continue_test {
        std::string_view input{};
        std::variant<i64, std::string> expected{};
    };

    std::array tests{
        continue_test{
                      R"(
        let x = 0;
        let y = 0;

        while (x < 5) {
            x = x + 1;

            if (x == 3) {
                continue;
            }

            y = y + 1;
        }

        y)",                       4
        },
        continue_test{"if (true) { continue; }", "continue statement is illegal in current context"},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};

        std::visit(
            [&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, i64>) {
                    test_int_object(*evaluated, val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    auto err{dynamic_cast<object::error&>(*evaluated)};
                    ASSERT_EQ(err.message, val);
                }
            },
            test.expected
        );
    }
}
