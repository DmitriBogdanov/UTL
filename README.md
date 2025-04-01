[<img src ="docs/images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="docs/images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="docs/images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="docs/images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

# UTL

**UTL** is a collection of small self-contained libraries that aim to provide a set of concise utilities aimed at prototyping with minimal boilerplate. Most of the modules were created during my work in gamedev and math research projects.

For the ease of integration, this library is distributed in a form of a **single header**, which can be found [here](./single_include/UTL.hpp).

All modules can also be downloaded [individually](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL).

## Design goals

Implementation of this library sets following design goals:

* **Header only**. Adding library to the project should be as simple as adding a header.
* **Concise**. Methods must require minimal boilerplate on user side.
* **Platform agnostic**. Implementation should be based around C++ standard and whenever OS-specific methods are unavoidable select appropriate implementation automatically.
* **Fully modular**. Every module should be independent of others and reside in a separate namespace. Modules together with their STL dependencies should be toggleable through a `#define`.

Secondary design goals also include:

* **Modern C++ style**. Usage of modern C++ idioms is preferred.
* **Usage examples**. Documentation should include usage examples for all major methods.
* **Test & benchmark coverage.** All modules should have appropriate unit test coverage, performance-focused implementations should be benchmarked against alternative approaches.

## Modules & documentation

| Module                                                     | Short description                                                    |
| ---------------------------------------------------------- | -------------------------------------------------------------------- |
| [**utl::bit**](./docs/module_bit.md)                       | Bit-twiddling, enum bitflags                                         |
| [**utl::enum_reflect**](./docs/module_enum_reflect.md)     | Enum reflection                                                      |
| [**utl::integral**](./docs/module_integral.md)             | Saturated math, safe integer casts, literals, rounding and etc.      |
| [**utl::json**](./docs/module_json.md)                     | JSON parsing, serializing & reflection                               |
| [**utl::log**](./docs/module_log.md)                       | Logging library                                                      |
| [**utl::math**](./docs/module_math.md)                     | Math-related utilities                                               |
| [**utl::mvl**](./docs/module_mvl.md)                       | Flexible API for vector and matrix operations                        |
| [**utl::parallel**](./docs/module_parallel.md)             | Thread pool, async tasks, parallel for, parallel reductions and etc. |
| [**utl::predef**](./docs/module_predef.md)                 | Detection of architectures, compilers, platforms and etc.            |
| [**utl::profiler**](./docs/module_profiler.md)             | Scope & expression profiling macros                                  |
| [**utl::progressbar**](./docs/module_progressbar.md)       | Progress bars for CLI apps                                           |
| [**utl::random**](./docs/module_random.md)                 | PRNGs & random number generation                                     |
| [**utl::shell**](./docs/module_shell.md)                   | Shell commands and temporary files                                   |
| [**utl::sleep**](./docs/module_sleep.md)                   | Precise sleep implementations                                        |
| [**utl::stre**](./docs/module_stre.md)                     | Efficient implementations of common string utils                     |
| [**utl::struct_reflect**](./docs/module_struct_reflect.md) | Struct reflection                                                    |
| [**utl::table**](./docs/module_table.md)                   | Drawing of LaTeX and ASCII tables                                    |
| [**utl::time**](./docs/module_time.md)                     | Floating-point time, timers, stopwatches, datetime                   |

## See also

* [How to include only specific modules](./docs/guide_selecting_modules.md)

* [Names reserved for implementation](./docs/guide_reserved_names.md)

* [Building tests & examples](./docs/guide_building_project.md)

## Requirements

* Requires **C++17** support
* Some modules require POSIX-compliant system (Linux) or Windows

## Third-party tools & libraries

While the library itself consists of a single header with no embedded dependencies, it was built and tested using a number of third-party tools and libraries, some of which are embedded in a repo.

| Tool                                                         | Version      | Used for                                                     |
| ------------------------------------------------------------ | ------------ | ------------------------------------------------------------ |
| [clang-format](https://clang.llvm.org/docs/ClangFormat.html) | **v.14.0.0** | Automatic code formatting                                    |
| [clangd](https://clangd.llvm.org)                            | **v.15.0.7** | Language server functionality                                |
| [CMake](https://cmake.org)                                   | **v.3.2.11** | Build system, [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) testing facilities |
| [GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) | **v.11.4.0** | [ASan](https://github.com/google/sanitizers/wiki/AddressSanitizer) and [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) instrumentation for the test environment |
| [cppcheck](https://github.com/danmar/cppcheck)               | **v.2.7**    | Static analysis                                              |

| Library                                                | Version            | License                                                                        | Used for                   | Embedded in repo |
| ------------------------------------------------------ | ------------------ | ------------------------------------------------------------------------------ | -------------------------- | ---------------- |
| [doctest](https://github.com/doctest/doctest)          | **v.2.4.11**       | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt)              | Unit testing               | ✔                |
| [nanobench](https://github.com/martinus/nanobench)     | **v.4.3.11**       | [MIT](https://github.com/martinus/nanobench/blob/master/LICENSE)               | Benchmarking               | ✔                |
| [nlohmann json](https://github.com/nlohmann/json)      | **v.3.11.3**       | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)               | Benchmark comparison       | ✔                |
| [PicoJSON](https://github.com/kazuho/picojson)         | **v.1.3.0**        | [BSD-2](https://github.com/kazuho/picojson/blob/master/LICENSE)                | Benchmark comparison       | ✔                |
| [RapidJSON](https://github.com/Tencent/rapidjson)      | **v.1.1.0**        | [MIT, BSD, JSON](https://github.com/Tencent/rapidjson/blob/master/license.txt) | Benchmark comparison       | ✔                |
| [JSONTestSuite](https://github.com/nst/JSONTestSuite/) | **commit 1ef36fa** | [MIT](https://github.com/nst/JSONTestSuite/blob/master/LICENSE)                | JSON Validation test suite | ✔                |

## Work in progress

* Old module reworks (`utl::timer` and `utl::table`);
* Examples for `utl::math`;
* Small API improvements for `utl::log`;
* Vector support for `utl::mvl`;
* Matrix concat functions for `utl::mvl`;
* Index span API for `utl::mvl`;
* Binary operator refactors for `utl::mvl`;
* Test coverage statistics.

## License

This project is licensed under the MIT License - see the [LICENSE.md](./LICENSE.md) for details.
