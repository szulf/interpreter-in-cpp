#include "object.h"
#include <sstream>
#include <variant>

namespace interp {

namespace object {

auto get_object_type_string(object_type obj) -> std::string_view {
    switch (obj) {
    case object_type::Integer:
        return "Integer";
    case object_type::Boolean:
        return "Boolean";
    case object_type::Null:
        return "Null";
    case object_type::ReturnValue:
        return "ReturnValue";
    case object_type::Error:
        return "Error";
    case object_type::Function:
        return "Function";
    }
}

environment::environment(const environment& other) {
    std::visit(
        [&](const auto& val) {
            if constexpr (std::is_same_v<std::unique_ptr<environment>, decltype(val)>) {
                outer = val ? val->clone() : nullptr;
            } else if constexpr (std::is_same_v<environment*, decltype(val)>) {
                outer = val;
            }
        },
        other.outer
    );

    for (const auto& [key, val] : other.store) {
        store[key] = val->clone();
    }
}

auto environment::operator=(const environment& other) -> environment& {
    if (this == &other) {
        return *this;
    }

    std::visit(
        [&](const auto& val) {
            if constexpr (std::is_same_v<std::unique_ptr<environment>, decltype(val)>) {
                outer = val->clone();
            } else if constexpr (std::is_same_v<environment*, decltype(val)>) {
                outer = val;
            }
        },
        other.outer
    );

    store.clear();
    for (const auto& [key, val] : other.store) {
        store[key] = val->clone();
    }

    return *this;
}

auto environment::get(const std::string& name) const -> const std::unique_ptr<object>* {
    try {
        return &store.at(name);
    } catch (const std::exception&) {
        return std::visit(
            [&](const auto& val) {
                if constexpr (std::is_same_v<std::unique_ptr<environment>, decltype(val)> ||
                              std::is_same_v<environment*, decltype(val)>) {
                    if (val) {
                        return val->get(name);
                    }
                }

                return nullptr;
            },
            outer
        );
    }
}

auto environment::set(const std::string& name, std::unique_ptr<object> val) -> const object& {
    store.insert_or_assign(name, std::move(val));
    return *store[name];
}

auto function::to_string() const -> std::string {
    std::stringstream ss{};
    ss << "fn(";
    for (u32 i = 0; const auto& p : parameters) {
        ss << p->to_string();
        if (i != parameters.size()) {
            ss << ", ";
        }
        i++;
    }
    ss << ") {\n" << body->to_string() << "\n}";

    return ss.str();
}

function::function(const function& other) : body{other.body->clone()}, env_outer{other.env_outer} {
    for (const auto& param : other.parameters) {
        parameters.emplace_back(param->clone());
    }
}

}

}
