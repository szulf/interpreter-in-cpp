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

TEST(eval, int_expression) {
    using namespace interp;

    struct int_test {
        std::string_view input{};
        i64 expected{};
    };

    static constexpr std::array tests{
        int_test{"5",  5 },
        int_test{"10", 10},
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
        bool_test{"true",  true },
        bool_test{"false", false},
    };

    for (const auto& test : tests) {
        auto evaluated{test_eval(test.input)};
        test_bool_object(*evaluated, test.expected);
    }
}
