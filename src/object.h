#pragma once

#include "ast.h"
#include "types.h"
#include <format>
#include <functional>
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
    String,
    Builtin,
    Array,
    Hash,
};

auto get_object_type_string(object_type obj) -> std::string_view;

class object {
public:
    virtual ~object() = default;

    virtual auto clone() const -> std::unique_ptr<object> = 0;

    virtual auto type() const -> object_type = 0;
    virtual auto to_string() const -> std::string = 0;
};

class hash_key {
public:
    auto operator==(const hash_key& other) const -> bool;

public:
    object_type type{};
    u64 value{};
};

class hashable {
public:
    virtual ~hashable() = default;

    virtual auto get_hash_key() const -> hash_key = 0;
};

class integer : public object, public hashable {
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

    auto get_hash_key() const -> hash_key override;

public:
    i64 value{};
};

class boolean : public object, public hashable {
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

    auto get_hash_key() const -> hash_key override;

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

    auto set(const std::string& name, std::unique_ptr<object> val) -> void;
    auto get(const std::string& name) const -> const std::unique_ptr<object>*;
    auto contains(const std::string& name) const -> bool;
    auto update(const std::string& name, std::unique_ptr<object> val) -> void;

    auto operator==(const environment& other) const -> bool;

public:
    std::unordered_map<std::string, std::unique_ptr<object>> store{};

    environment* outer{};
    std::vector<std::unique_ptr<environment>> envs_inner{};
};

class function : public object {
public:
    function(std::vector<std::unique_ptr<ast::expression>> params, std::unique_ptr<ast::statement> b, environment& e)
        : parameters{std::move(params)}, body{std::move(b)}, env_outer{e} {}
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
    environment& env_outer;
};

class string : public object, public hashable {
public:
    string() {}
    string(const std::string& val) : value{val} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<string>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::String;
    }

    inline auto to_string() const -> std::string override {
        return std::format("\"{}\"", value);
    }

    auto get_hash_key() const -> hash_key override;

public:
    std::string value{};
};

using builtin_function = std::function<std::unique_ptr<object>(std::vector<std::unique_ptr<object>>)>;

class builtin : public object {
public:
    builtin() {}
    builtin(const builtin_function& f) : fn{f} {}

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<builtin>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::String;
    }

    inline auto to_string() const -> std::string override {
        return "builtin function";
    }

public:
    builtin_function fn;
};

class array : public object {
public:
    array() {}
    array(const array& other);

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<array>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Array;
    }

    auto to_string() const -> std::string override;

public:
    std::vector<std::unique_ptr<object>> elements{};
};

}
}

template <>
struct std::hash<interp::object::hash_key> {
    interp::usize operator()(const interp::object::hash_key& hk) const noexcept {
        interp::usize h1 = std::hash<interp::u64>{}(hk.value);
        interp::usize h2 = std::hash<interp::u64>{}(static_cast<interp::u64>(hk.type));

        return h1 ^ (h2 << 1);
    }
};

namespace interp {

namespace object {

class hash : public object {
public:
    hash() {}
    hash(const hash& other);

    inline auto clone() const -> std::unique_ptr<object> override {
        return std::make_unique<hash>(*this);
    }

    inline auto type() const -> object_type override {
        return object_type::Hash;
    }

    auto to_string() const -> std::string override;

public:
    std::unordered_map<hash_key, std::pair<std::unique_ptr<object>, std::unique_ptr<object>>> pairs{};
};

}
}
