# Interpreter written in C++

based on a book "writing an interpreter in go"

## Usage

The language contains features like
- javascript like objects
- arrays, strings, integers, booleans, null
- functions and builtin functions like 'len()' or 'puts()'

### Example program

For  examples look in the example/ directory

## Building

Clone the repo
```bash
git clone https://github.com/szulf/interpreter-in-cpp.git && cd interpreter-in-cpp
```

Build the cmake cache file
```bash
mkdir build
cmake -B build -S .
```

Build the project
```bash
cmake --build build
```

## Issues(not going to be fixed)
- on some errors the compiler segfaults instead of printing error(stack overflow, maybe others)
