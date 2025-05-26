#pragma once

#include "ast.h"
#include "types.h"
#include <format>
#include <memory>
#include <string>
#include <unordered_map>

namespace interp {

namespace object {

enum class object_type : u8 {
    Integer,
    Boolean,
    Null,
    ReturnValue,
    Error,
    Function,
};

auto get_object_type_string(object_type obj) -> std::string_view;

class object {
public:
    virtual ~object() = default;

    virtual auto clone() const -> std::unique_ptr<object> = 0;

    virtual auto type() const -> object_type = 0;
    virtual auto to_string() const -> std::string = 0;
};

class integer : public object {
public:
    integer() {}
    integer(i64 val) : value{val} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<integer>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Integer;
    }

    inline auto to_string() const -> std::string override {
        return std::format("{}", value);
    }

public:
    i64 value{};
};

class boolean : public object {
public:
    boolean() {}
    boolean(bool val) : value{val} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<boolean>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Boolean;
    }

    inline auto to_string() const -> std::string override {
        return std::format("{}", value);
    }

public:
    bool value{};
};

class null : public object {
public:
    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<null>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Null;
    }

    inline auto to_string() const -> std::string override {
        return "null";
    }
};

class return_value : public object {
public:
    return_value() {}
    return_value(std::unique_ptr<object> val) : value{std::move(val)} {}

    return_value(const return_value& val) : value{val.clone()} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<return_value>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::ReturnValue;
    }

    inline auto to_string() const -> std::string override {
        return value->to_string();
    }

public:
    std::unique_ptr<object> value{};
};

class error : public object {
public:
    error() {}
    error(std::string_view msg) : message{msg} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<error>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Error;
    }

    inline auto to_string() const -> std::string override {
        return "error: " + message;
    }

public:
    std::string message{};
};

class environment {
public:
    environment() {}
    environment(environment* outer) : outer{outer} {}

    auto get(const std::string& name) const -> const std::unique_ptr<object>*;
    auto set(const std::string& name, std::unique_ptr<object> val) -> const object&;

public:
    environment* outer{};
    std::unordered_map<std::string, std::unique_ptr<object>> store{};
};

class function : public object {
public:
    function(std::vector<std::unique_ptr<ast::expression>> params, std::unique_ptr<ast::statement> b, environment& e)
        : parameters{std::move(params)}, body{std::move(b)}, env{&e} {}
    function(const function& other);

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<function>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Function;
    }

    auto to_string() const -> std::string override;

public:
    std::vector<std::unique_ptr<ast::expression>> parameters{};
    std::unique_ptr<ast::statement> body{};
    environment* env;
};

}
}
