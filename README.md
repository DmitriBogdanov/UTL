[<img src ="docs/images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="docs/images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="docs/images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="docs/images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="docs/images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# UTL

**UTL** is a collection of small self-contained libraries aimed at prototyping with minimal boilerplate. Most of the modules were created during my work in gamedev and math research projects.

For the ease of integration, all libraries are distributed as **individual headers**, which can be found [**here**](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL).

The entire collection can also be downloaded as an [**amalgamated single-header**](./single_include/UTL.hpp).

## Design goals

Implementation of this library sets following design goals:

* **Easy integration**. Adding libraries to the project should be as simple as including their corresponding headers.
* **Concise syntax**. Library usage should require minimal boilerplate on user side.
* **Platform agnostic.** Implementation should be based around the C++ standard. Platform-specific features might be provided as optional addons.
* **Non-intrusive.** Every library should reside in its own namespace, use its own macro prefix and not introduce any invasive identifiers.

Quality goals include:

* **Modern C++ style**. Usage of modern C++ idioms is heavily preferred.
* **Usage examples**. Documentation should include  practical usage examples for all major methods.
* **Test & benchmark coverage.** All modules should have appropriate unit test coverage, performance-focused implementations should be benchmarked against alternative approaches.
* **Easy to build.** All headers, tests, benchmarks and examples should build under the strictest warning levels and with minimal tooling.

## Modules & documentation

| Module                                                     | Short description                                                    |
| ---------------------------------------------------------- | -------------------------------------------------------------------- |
| [**utl::bit**](./docs/module_bit.md)                       | Bit-twiddling, enum bitflags                                         |
| [**utl::enum_reflect**](./docs/module_enum_reflect.md)     | Enum reflection                                                      |
| [**utl::integral**](./docs/module_integral.md)             | Saturated math, safe integer casts, literals, rounding and etc.      |
| [**utl::json**](./docs/module_json.md)                     | JSON parsing, serializing & reflection                               |
| [**utl::log**](./docs/module_log.md)                       | Logging library                                                      |
| [**utl::math**](./docs/module_math.md)                     | Additional math functions                                            |
| [**utl::mvl**](./docs/module_mvl.md)                       | Generalized matrix & vector containers                               |
| [**utl::parallel**](./docs/module_parallel.md)             | Thread pool, async tasks, parallel for, parallel reductions and etc. |
| [**utl::predef**](./docs/module_predef.md)                 | Detection of architectures, compilers, platforms and etc.            |
| [**utl::profiler**](./docs/module_profiler.md)             | Call graph & thread profiling                                        |
| [**utl::progressbar**](./docs/module_progressbar.md)       | Progress bars for CLI apps                                           |
| [**utl::random**](./docs/module_random.md)                 | PRNGs & random number generation                                     |
| [**utl::shell**](./docs/module_shell.md)                   | Shell commands and temporary files                                   |
| [**utl::sleep**](./docs/module_sleep.md)                   | Precise sleep implementations                                        |
| [**utl::stre**](./docs/module_stre.md)                     | Efficient implementations of common string utils                     |
| [**utl::struct_reflect**](./docs/module_struct_reflect.md) | Struct reflection                                                    |
| [**utl::table**](./docs/module_table.md)                   | Drawing of LaTeX and ASCII tables                                    |
| [**utl::time**](./docs/module_time.md)                     | Floating-point time, timers, stopwatches, datetime                   |

## See also

* [How to include only specific modules when using amalgamated header](./docs/guide_selecting_modules.md)
* [Fetching the library with CMake `FetchContent()`](./docs/guide_fetching_library.md)
* [Names reserved for implementation](./docs/guide_reserved_names.md)
* [Building tests & benchmarks](./docs/guide_building_project.md)
* [Semantic versioning](./docs/guide_versioning.md)

## Requirements

* Requires **C++17** support

## Developer toolchain

While the library itself consists of a single header with no embedded dependencies, it was built and tested using a number of third-party tools and libraries, some of which are embedded in the repo.

| Tool                                                                                                   | Version      | Used for                                                                                                                                                         |
| ------------------------------------------------------------------------------------------------------ | ------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [clang-format](https://clang.llvm.org/docs/ClangFormat.html)                                           | **v.14.0.0** | Automatic code formatting                                                                                                                                        |
| [clangd](https://clangd.llvm.org)                                                                      | **v.15.0.7** | Language server functionality                                                                                                                                    |
| [CMake](https://cmake.org)                                                                             | **v.3.2.11** | Build system, [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) testing facilities                                                                |
| [GCC](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)                                 | **v.11.4.0** | [ASan](https://github.com/google/sanitizers/wiki/AddressSanitizer) and [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) test instrumentation |
| [cppcheck](https://github.com/danmar/cppcheck)                                                         | **v.2.7**    | Static analysis                                                                                                                                                  |
| [GitHub Actions](https://docs.github.com/en/actions/about-github-actions/understanding-github-actions) | **latest**   | Multi-platform [CI testing](https://docs.github.com/en/actions/about-github-actions/about-continuous-integration-with-github-actions)                            |

| Library                                                       | Version            | License                                                                        | Used for                   | Embedded in repo |
| ------------------------------------------------------------- | ------------------ | ------------------------------------------------------------------------------ | -------------------------- | ---------------- |
| [doctest](https://github.com/doctest/doctest)                 | **v.2.4.11**       | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt)              | Unit testing               | ✔                |
| [nanobench](https://github.com/martinus/nanobench)            | **v.4.3.11**       | [MIT](https://github.com/martinus/nanobench/blob/master/LICENSE)               | Benchmarking               | ✔                |
| [nlohmann json](https://github.com/nlohmann/json)             | **v.3.11.3**       | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)               | Benchmark comparison       | ✔                |
| [PicoJSON](https://github.com/kazuho/picojson)                | **v.1.3.0**        | [BSD-2](https://github.com/kazuho/picojson/blob/master/LICENSE)                | Benchmark comparison       | ✔                |
| [RapidJSON](https://github.com/Tencent/rapidjson)             | **v.1.1.0**        | [MIT, BSD, JSON](https://github.com/Tencent/rapidjson/blob/master/license.txt) | Benchmark comparison       | ✔                |
| [JSONTestSuite](https://github.com/nst/JSONTestSuite/)        | **commit 1ef36fa** | [MIT](https://github.com/nst/JSONTestSuite/blob/master/LICENSE)                | JSON Validation test suite | ✔                |
| [BS::thread_pool](https://github.com/bshoshany/thread-pool)   | **v.5.0.0**        | [MIT](https://github.com/bshoshany/thread-pool/blob/master/LICENSE.txt)        | Benchmark comparison       | ✔                |
| [progschj/ThreadPool](https://github.com/progschj/ThreadPool) | **commit 9a42ec1** | [zlib](https://github.com/progschj/ThreadPool/blob/master/COPYING)             | Benchmark comparison       | ✔                |

## Work in progress

* Reach zero warnings at MSVC `/W4`
* Possibly provide PCH target for internal and external use
* `utl::table` rewrite
* `utl::log` formatting rework
* `utl::mvl` large scale refactor and v1.0.0 release
* `utl::random` CBPRNGs and fixed-entropy seeding sequence
* Test coverage statistics

## License

This project is licensed under the MIT License - see the [LICENSE.md](./LICENSE.md) for details.
