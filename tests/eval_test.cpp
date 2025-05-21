#include <gtest/gtest.h>

#include "eval.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "types.h"

static auto test_eval(std::string_view input) -> std::unique_ptr<interp::object::object> {
    using namespace interp;

    lexer::lexer l{input};
    parser::parser p{l};
    auto program{p.parse_program()};

    return eval::eval(program);
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
