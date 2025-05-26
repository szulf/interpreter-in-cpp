#include "object.h"
#include <print>
#include <sstream>
#include <stdexcept>

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

auto environment::get(const std::string& name) const -> const std::unique_ptr<object>* {
    try {
        return &store.at(name);
    } catch (const std::out_of_range&) {
        if (outer) {
            return outer->get(name);
        }
        return nullptr;
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

function::function(const function& other) : body{other.body->clone()}, env{other.env} {
    for (const auto& param : other.parameters) {
        parameters.emplace_back(param->clone());
    }
}

}

}
