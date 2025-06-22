# Building tests & examples

[<- back to README.md](..)

This project uses [CMake](https://cmake.org) build system.

Unit testing is done in terms of [doctest](https://github.com/doctest/doctest) framework together with **CMake** testing facilities ([CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)). See [`tests/`](https://github.com/DmitriBogdanov/UTL/tree/master/tests).

Benchmarks are implemented using the [nanobench](https://github.com/martinus/nanobench) library. See [`benchmarks/`](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks).

All tests and benchmarks are compiled with `-Wall -Wextra -Wpedantic -Werror`. Tests also use sanitizers provided by the GCC / LLVM toolchain.

## Building with a script

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/UTL.git &&
cd "UTL/"
```

Configure & build the project:

```bash
bash actions.sh clear config build
```

Run all tests:

```bash
bash actions.sh test
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```

## Building manually

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/UTL.git &&
cd "UTL/"
```

Configure **CMake**:

```bash
cmake --preset gcc
```

Build the project:

```bash
bash "bash/create_single_header.sh" &&
cmake --build --preset gcc
```

Run tests:

```bash
ctest --build --preset gcc
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```

## Configuring build

**Compiler**, **flags** and **CTest arguments** can be configured in [`CMakePresets.json`](./../CMakePresets.json).

Alternatively, it is possible to override a specific variable from an existing preset, for example, to specify `g++13` instead of regular `g++`:

```bash
cmake --preset gcc -D CMAKE_CXX_COMPILER="g++13"
```
