#pragma once

#include "ast.h"
#include "object.h"

namespace interp {

namespace eval {

auto eval(ast::node& node) -> std::unique_ptr<object::object>;

}

}
