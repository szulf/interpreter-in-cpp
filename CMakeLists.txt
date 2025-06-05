cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(interp
    VERSION 1.0
    LANGUAGES CXX
)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

SET(SRC_FILES
    ${SRC_DIR}/types.h
    ${SRC_DIR}/token.cpp ${SRC_DIR}/token.h
    ${SRC_DIR}/lexer.cpp ${SRC_DIR}/lexer.h
    ${SRC_DIR}/helpers.cpp ${SRC_DIR}/helpers.h
    ${SRC_DIR}/repl.cpp ${SRC_DIR}/repl.h
    ${SRC_DIR}/ast.cpp ${SRC_DIR}/ast.h
    ${SRC_DIR}/parser.cpp ${SRC_DIR}/parser.h
    ${SRC_DIR}/object.cpp ${SRC_DIR}/object.h
    ${SRC_DIR}/eval.cpp ${SRC_DIR}/eval.h
)

add_executable(interp
    src/main.cpp
    ${SRC_FILES}
)

add_subdirectory(tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -Werror -Wextra -Wconversion -Wuninitialized -Wunused -fsanitize=address -fsanitize=leak")
