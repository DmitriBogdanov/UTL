# Project road-map

This file contains a rough road-map of planned features.

## Future C++20 migration

### Requirements

**C++20** should have a statistical majority of users. As of now that title belong to **C++17**.

### Minimal task list

Before migration following tasks are to be finished:
- Finish all modules below version `1.0.0`
- Standardize benchmarks (using standard `benchmark.hpp`) **(done ✔)**
- Standardize tests (using standard `test.hpp`) **(done ✔)**
- Write unit tests for all reasonably testable modules
- Add all documentation examples to the build process
- Switch building to CMake presets **(done ✔)**
- Add CI workflows for all major platforms
- Ensure full documentation coverage

Once this is done, the current branch can be archived as `cpp17` branch, while `master` moves on to C++20.

### Migration refactors

| Refactor                                                     | Expected scope                                               | Notes                                                        | Completion |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ---------- |
| Replace SFINAE with concepts                                 | `mvl` everything, `math` constraints, `log` stringification, `integral`, `random`, almost everything really | Do a double take on function requirements, perhaps specify some even more. | ✘          |
| Replace `std::thread` with `std::jthread`                    | `parallel::ThreadPool`                                       |                                                              | ✘          |
| Replace class operators `<, >, ==, !=` with a spaceship operator `<=>` | `integral`                                                   |                                                              | ✘          |
| Add `[[likely]]` and `[[unlikely]]` attributes where appropriate | `json` exceptions, `mvl` bound checking                      |                                                              | ✘          |
| Use `__VA_OPT__` to remove the need for trailing comma support by the compiler | `predef`                                                     |                                                              | ✘          |
| Replace some occurrences of `const std::vector<>` with `std::span<>` |                                                              |                                                              | ✘          |
| Remove/rename some constants on `utl::math` as they are now part of `std` | `math`                                                       |                                                              | ✘          |
| Remove `stre::start_with()` and `stre::ends_with()` as they are now part of `std` | `stre`                                                       |                                                              | ✘          |
| Replace map/set `.count()` with `.contains()`                | `json`                                                       |                                                              | ✘          |
| Add `constinit` to static variables that can be initialized from `constexpr` values |                                                              |                                                              | ✘          |
| Add `using enum` to appropriate switch-cases                 |                                                              |                                                              | ✘          |
| Replace macros that use `__FILE__`, `__LINE__`, `__func` with templates using `std::source_location` | `log` macro API                                              |                                                              | ✘          |
| Deprecate functions that get added into the standard library (`rotl()`, `in_range()`, integral comparators, `to_underlying()`, `starts_with()`, `ends_with()`, `ssize()`) | `random`, `integral`, `stre`, `math`                         | See std `<bit>`, `<compare>`, `<string>`                     | ✘          |
| Deprecate math constant that get added into a standard library | `math`                                                       | See std `<numbers>`                                          | ✘          |
| Replace some casts with explicit `std::bit_cast<>()`         | `random`, `integral`                                         |                                                              | ✘          |
| Use `std::endian` for additional compilation summary         | `predef`                                                     |                                                              | ✘          |
| Switch from `C` way of getting dates to `<chrono>` calendar additions | `log`, `time`                                                |                                                              | ✘          |

### Useful links

- [Listing of features added in each standard from C++11 to C++20](https://github.com/AnthonyCalandra/modern-cpp-features)
- [Cppreference for new language features](https://en.cppreference.com/w/cpp/20)
