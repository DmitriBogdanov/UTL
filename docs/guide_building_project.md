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

## Configuring build

**Compiler** and **CTest** flags can be selected in [`bash/variables.sh`](./../bash/variables.sh). To do so, edit following lines at the end of the script:

```bash
compiler="g++"
test_flags="--rerun-failed --output-on-failure --timeout 60"
```

**Compilation flags** can be changed in [`tests/CMakeLists.txt`](./../tests/CMakeLists.txt) and [`benchmarks/CMakeLists.txt`](./../benchmarks/CMakeLists.txt).

## Building manually

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/UTL.git &&
cd "UTL/"
```

Configure **CMake**:

```bash
cmake -D CMAKE_CXX_COMPILER=g++ -B "build/" -S .
```

Build the project:

```bash
bash "bash/create_single_header.sh"
cmake --build "build/"
```

Run tests:

```bash
cd "build/tests/" &&
ctest --rerun-failed --output-on-failure --timeout 60 &&
cd ..
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```
