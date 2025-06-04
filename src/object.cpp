#include "object.h"
#include <sstream>

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

    return "wut";
}

auto environment::get(const std::string& name) const -> const std::unique_ptr<object>* {
    if (store.contains(name)) {
        return &store.at(name);
    }

    if (outer) {
        return outer->get(name);
    }

    return nullptr;
}

auto environment::set(const std::string& name, std::unique_ptr<object> val) -> void {
    store[name] = std::move(val);
}

auto environment::operator==(const environment& other) const -> bool {
    return store == other.store && outer == other.outer && envs_inner == other.envs_inner;
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
