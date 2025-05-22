#include "object.h"

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
    }
}

auto environment::get(const std::string& name) const -> const std::unique_ptr<object>& {
    return store.at(name);
}

auto environment::set(const std::string& name, std::unique_ptr<object> val) -> const object& {
    store.insert_or_assign(name, std::move(val));
    return *store[name];
}

}

}
