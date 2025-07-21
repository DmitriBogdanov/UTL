# Project road-map

This file contains a rough road-map of planned features.

## Future C++20 migration

### Requirements

**C++20** should have a statistical majority of users. As of now that title belongs to **C++17**.

### Minimal task list

Before migration following tasks are to be finished:
- Finish all modules below version `1.0.0`
- Standardize benchmarks (using standard `benchmarks/common.hpp`) **(done ✔)**
- Standardize tests (using standard `tests/common.hpp`) **(done ✔)**
- Write unit tests for all reasonably testable modules
- Add all documentation examples to the build process **(done ✔)**
- Switch building to CMake presets **(done ✔)**
- Add CI workflows for all major platforms **(done ✔)**
- Ensure full documentation coverage **(done ✔)**

Once this is done, the current branch can be archived as `cpp17` branch, while `master` moves on to C++20.

### Migration refactors

| Refactor                                                     | Expected scope                                               | Motivation                                                   | Completion |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ---------- |
| Replace SFINAE with concepts                                 | `mvl` everything, `math` constraints, `log` stringification, `integral`, `random`, almost everything really | Better clarity, improved compile times, better error messages | ✘          |
| Do a pass on possible `constexpr` expansion for all the functions that got `constexpr` support | ?                                                            | More `constexpr`, better testing and UB guarantees           | ✘          |
| Replace some stream-based formatting with `std::format`      | `log`, possibly others                                       | Better performance                                           | ✘          |
| Replace `std::thread` with `std::jthread`                    | `parallel`                                                   | Stop token support, automatic joining                        | ✘          |
| See if some uses of `std::conditional_variable` waiting can be replaced with `std::atomic` wait | `parallel`                                                   | Possible performance enhancement                             | ✘          |
| Replace class operators `<, >, ==, !=` with a spaceship operator `<=>` | ?                                                            | More generic                                                 | ✘          |
| Add `[[likely]]` and `[[unlikely]]` attributes where appropriate | `json` exceptions, `mvl` bound checking                      | Possible performance enhancement                             | ✘          |
| Use `__VA_OPT__` to remove the need for trailing comma support by the compiler | ?                                                            | Less reliance of compiler specifics                          | ✘          |
| Replace some occurrences of `const std::vector<>` with `std::span<>` | ?                                                            | Cleaner & faster API                                         | ✘          |
| Replace map/set `.count()` with `.contains()`                | `json`                                                       | Better clarity                                               | ✘          |
| Add `constinit` to static variables that can be initialized from `constexpr` values | ?                                                            | Possible performance enhancement                             | ✘          |
| Replace macros that use `__FILE__`, `__LINE__`, `__func` with templates using `std::source_location` | `log`                                                        | Less macro usage                                             | ✘          |
| Replace some casts with explicit `std::bit_cast<>()`         | `random`, `integral`                                         | Better clarity, possibly performance enhancement in some cases | ✘          |
| Use `std::endian` for additional compilation summary         | `predef`                                                     | More features                                                | ✘          |
| Switch from `C` way of getting dates to `<chrono>` calendar additions | `log`, `time`                                                | Better performance, cleared and more powerful system         | ✘          |
| Remove `stre::start_with()` and `stre::ends_with()` as they are now a part of `std` | `stre`                                                       | **!** Might be be worth leaving to prevent API breakage      | ✘          |
| Remove/rename some constants on `utl::math` as they are now a part of `std` | `math`                                                       | **!** Might be be worth leaving to prevent API breakage      | ✘          |
| Remove heterogeneous integer comparison and `in_range()` from `utl::integral` as it is now a part of `std` | `integral`                                                   | **!** Might be be worth leaving to prevent API breakage      | ✘          |
| Remove `to_underlying()` as it is now a part of `std`        | `enum_reflect()`                                             | **!** Might be be worth leaving to prevent API breakage      | ✘          |
| Replace `rotl()` and `popcount()` with `std` functions       | `random`                                                     | Possible performance enhancement, needs benchmarking         | ✘          |

### Useful links

- [Listing of features added in each standard from C++11 to C++20](https://github.com/AnthonyCalandra/modern-cpp-features)
- [Cppreference for new language features](https://en.cppreference.com/w/cpp/20)
