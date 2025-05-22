#pragma once

#include "ast.h"
#include "object.h"

namespace interp {

namespace eval {

auto eval(ast::node& node, object::environment& env) -> std::unique_ptr<object::object>;

}

}
