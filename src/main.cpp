#include "repl.h"

#include <iostream>
#include <print>

int main() {
    std::println("Hello user! This is the interp programming language!");
    std::println("Feel free to type in commands");

    interp::repl::start(std::cin, std::cout);

    return 0;
}
