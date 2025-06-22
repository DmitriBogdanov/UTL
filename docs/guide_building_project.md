# Building tests & examples

[<- back to README.md](..)

This project uses [CMake](https://cmake.org) build system with [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) as a main way of managing configuration.

Unit testing is done in terms of [doctest](https://github.com/doctest/doctest) framework together with **CMake** testing facilities ([CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)). See [`tests/`](https://github.com/DmitriBogdanov/UTL/tree/master/tests).

Benchmarks are implemented using the [nanobench](https://github.com/martinus/nanobench) library. See [`benchmarks/`](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks).

All tests and benchmarks are compiled with `-Wall -Wextra -Wpedantic -Werror`. Tests also use sanitizers provided by the GCC / LLVM toolchain.

## Building with CMake

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
ctest --preset gcc
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```

## Building with a script

To reduce the tedium of entering verbose commands during development, this repo provides [`actions.sh`](./../actions.sh) script, containing shortcuts for all the actions above set up for `gcc` preset.


For example, we can clear previous build (if present), configure, build and run tests with a single command:

```bash
bash actions.sh clear config build test
```

## Configuring build

**Compiler**, **flags** and **CTest arguments** are specified in [`CMakePresets.json`](./../CMakePresets.json).

The canonical way of configuring local environment is `CMakeUserPresets.json`, use existing presets as a reference.

Alternatively, it is possible to override a specific variable from an existing preset, for example, to specify `g++13` instead of regular `g++` do:

```bash
cmake --preset gcc -D CMAKE_CXX_COMPILER="g++13"
```

