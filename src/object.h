#pragma once

#include "types.h"
#include <format>
#include <string>

namespace interp {

namespace object {

enum class object_type : u8 {
    Integer,
    Boolean,
    Null,
};

auto get_object_type_string(object_type obj) -> std::string_view;

class object {
public:
    virtual ~object() = default;

    virtual auto type() const -> object_type = 0;
    virtual auto inspect() const -> std::string = 0;
};

class integer : public object {
public:
    integer() {}
    integer(i64 val) : value{val} {}

    inline auto type() const -> object_type override {
        return object_type::Integer;
    }

    inline auto inspect() const -> std::string override {
        return std::format("{}", value);
    }

public:
    i64 value{};
};

class boolean : public object {
public:
    boolean() {}
    boolean(bool val) : value{val} {}

    inline auto type() const -> object_type override {
        return object_type::Boolean;
    }

    inline auto inspect() const -> std::string override {
        return std::format("{}", value);
    }

public:
    bool value{};
};

class null : public object {
public:
    inline auto type() const -> object_type override {
        return object_type::Null;
    }

    inline auto inspect() const -> std::string override {
        return "null";
    }
};

}
}
