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
    eval_test.cpp
    object_test.cpp
    ${SRC_FILES}
)

target_include_directories(interp_tests PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(interp_tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(interp_tests)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -Werror -Wextra -Wconversion -Wuninitialized -Wunused -fsanitize=address -fsanitize=leak")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -Werror -Wextra -Wconversion -Wuninitialized -Wunused")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

