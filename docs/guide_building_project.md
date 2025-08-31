# Building tests & examples

[<- back to README.md](..)

This project uses [CMake](https://cmake.org) build system with [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) as a main way of managing platform-dependent configuration.

All [benchmarks](../benchmarks), [tests](../tests) and [examples](../examples) are split into granular targets following a similar directory structure, these targets are built as a part of the [CI pipeline](https://docs.github.com/en/actions/concepts/overview/about-continuous-integration-with-github-actions) specified in GitHub [workflows](../.github/workflows).

For tests we use a built-in CMake test runner [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).

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

To reduce the tedium of entering verbose commands during development, this repo provides [`actions.sh`](../actions.sh) script, containing shortcuts for all the actions above set up for `gcc` preset.


For example, we can clear previous build (if present), configure, build and run tests with a single command:

```bash
bash actions.sh clear config build test
```

## Configuring build

**Compiler**, **flags** and **CTest arguments** are specified in [`CMakePresets.json`](../CMakePresets.json).

The canonical way of configuring local environment is `CMakeUserPresets.json`, use existing presets as a reference.

Alternatively, it is possible to override specific variables from an existing preset, for example, to specify `g++13` instead of regular `g++` use following configuration:

```bash
cmake --preset gcc -D CMAKE_CXX_COMPILER="g++13"
```

## Notes on the toolchain

All developer targets are compiled with `-Wall -Wextra -Wpedantic -Werror` (or their MSVC equivalents). Tests also use sanitizers provided by the GCC / LLVM toolchain.

CI pipeline is set up to test all 3 major compilers (`GCC`, `clang`, `MSVC`) with various standard libs (`libstdc++`, `libc++`, `STL`) on different operating systems (`Ubuntu`, `MacOS`, `Windows`). All documented examples also double as tests to ensure their validity.

For testing we use [doctest](https://github.com/doctest/doctest) framework wrapped in [`tests/common.hpp`](../tests/common.hpp).

For benchmarks we use [nanobench](https://github.com/martinus/nanobench) framework wrapped in [`benchmarks/common.hpp`](../benchmarks/common.hpp)
