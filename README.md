# Interpreter written in C++

based on a book "writing an interpreter in go"

## Usage

The language contains features like
- javascript like objects
- arrays, strings, integers, booleans, null
- functions and builtin functions like 'len()' or 'puts()'

### Example program
```
let x = "some text";
let double_len = fn(str) {
    let l = len(str)
    return l * l;
}

puts(double_len(x))
```

or

```
let arr = [];
let x = 0;
while (x < 5) {
    let a = {x: x * x};
    puts(a);
    arr = push(arr, a);
    x = x + 1;
}

puts(arr);
```
(loops are not supported as of now)
(assignment to already existing variables is not supported as of now)

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
- on some errors the compiler segfaults instead of printing error(stack overflow, parser errors)
