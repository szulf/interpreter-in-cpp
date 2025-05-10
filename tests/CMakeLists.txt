cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(interp_tests
    VERSION 1.0
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(interp_tests
    lexer_test.cpp
    parser_test.cpp
    ast_test.cpp
    ${SRC_FILES}
)

target_link_libraries(interp_tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(interp_tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -Werror -Wconversion -Wuninitialized -Wunused -fsanitize=address -fsanitize=leak")
