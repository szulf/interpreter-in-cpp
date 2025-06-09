#include "object.h"
#include <functional>
#include <sstream>
#include <utility>

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
    case object_type::String:
        return "String";
    case object_type::Builtin:
        return "Builtin";
    case object_type::Array:
        return "Array";
    case object_type::Hash:
        return "Hash";
    }

    std::unreachable();
}

auto hash_key::operator==(const hash_key& other) const -> bool {
    return type == other.type && value == other.value;
}

auto integer::get_hash_key() const -> hash_key {
    return hash_key{object_type::Integer, static_cast<u64>(value)};
}

auto boolean::get_hash_key() const -> hash_key {
    return hash_key{object_type::Boolean, static_cast<u64>(value)};
}

auto string::get_hash_key() const -> hash_key {
    static std::hash<std::string> hasher{};

    return hash_key{object_type::String, hasher(value)};
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
    for (const auto& p : parameters) {
        ss << p->to_string();
        if (p != parameters.back()) {
            ss << ", ";
        }
    }
    ss << ") {\n" << body->to_string() << "\n}";

    return ss.str();
}

function::function(const function& other) : body{other.body->clone()}, env_outer{other.env_outer} {
    for (const auto& param : other.parameters) {
        parameters.emplace_back(param->clone());
    }
}

array::array(const array& other) {
    for (const auto& elem : other.elements) {
        elements.emplace_back(elem->clone());
    }
}

auto array::to_string() const -> std::string {
    std::stringstream ss{};

    ss << "[";
    for (const auto& elem : elements) {
        ss << elem->to_string();
        if (elem != elements.back()) {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str();
}

hash::hash(const hash& other) {
    for (const auto& [key, val] : other.pairs) {
        pairs[key] = std::make_pair(val.first->clone(), val.second->clone());
    }
}

auto hash::to_string() const -> std::string {
    std::stringstream ss{};

    ss << "{";
    for (u32 i{0}; const auto& [k, val] : pairs) {
        ss << val.first->to_string() << ": " << val.second->to_string();
        if (i != pairs.size() - 1) {
            ss << ", ";
        }
    }
    ss << "}";

    return ss.str();
}

}

}
