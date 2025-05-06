#include "helpers.h"

#include <cctype>

namespace interp {

namespace helpers {

bool is_letter(i8 ch) {
    return std::isalpha(ch) || ch == '_';
}

}

}
