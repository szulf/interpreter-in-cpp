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
    }
}

}

}
