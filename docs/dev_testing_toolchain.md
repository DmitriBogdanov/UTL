# Testing toolchain

This file contains a high-level description of the toolchain and testing methodology.

## GCC & LLVM toolchain

### Motivation

We need a robust set of toolchain options to detect correctness and security issues.

GCC and LLVM toolchains are highly compatible and give us a good set of pedantic flags, sanitizers and coverage options.

### Compiler flags

| Flags                               | Motivation                                                   |
| ----------------------------------- | ------------------------------------------------------------ |
| `-O0`                               | No optimization makes compilation a bit faster               |
| `-g3`                               | Leaves as much debug info as possible                        |
| `-Wall -Wextra -Wpedantic -Werror`  | More warnings, treat warnings as errors                      |
| `-fsanitize=undefined,address,leak` | Enable sanitizers                                            |
| `-fno-sanitize-recover=all`         | Sanitizers should stop execution when  triggered  (which fails the test), default behavior is to continue |
| `--coverage`                        | Adds instrumentation for coverage analysis, for GCC works as an alias for `-fprofile-arcs -ftest-coverage` when compiling and `-lgcov` when linking |

### Linker flags

| Flag                                | Motivation                                                   |
| ----------------------------------- | ------------------------------------------------------------ |
| `-fsanitize=undefined,address,leak` | Sanitizers require linking against corresponding instrumentation libs |
| `--coverage`                        | Coverage analysis requires linking against corresponding instrumentation lib |

### Useful links

- [GCC debugging options](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html)
- [GCC instrumentation options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)
- [Clang diagnostic flags](https://clang.llvm.org/docs/DiagnosticsReference.html)
- [Clang ASan](https://clang.llvm.org/docs/AddressSanitizer.html)
- [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)

### Other useful flags

| Flag                      | Usage                                                        |
| ------------------------- | ------------------------------------------------------------ |
| `-fno-omit-frame-pointer` | Doesn't matter with `-O0`, prohibits optimization than makes stack traces harder to analyze |

## CMake presets

### Motivation

We need a clean way to build and test project for multiple platforms, compilers and configurations.

[CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) were added as an intended solution for this exact problem.

### Problems

While on first glance presets seem like a perfect solution for configuration management, they have several flaws complicating their usage in practice:

- Combinatorial explosion of configurations due to the lack of `--preset` merging (see [issue](https://gitlab.kitware.com/cmake/cmake/-/issues/22538))
- Strong association between build / test presets and configure presets, which has no real reason to exist
- Inability comment `CMakePresets.json` makes it difficult to keep track of different build quirks

### Summary

Despite all the issues, presets are still one of the few more-or-less sane ways of handling cross-platform building.

All platform-specific logic can be neatly enough isolated into `CMakePresets.json`.

### Useful links

- [CMake presets documentation](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [`cmake-init` as an example of robust preset management](https://github.com/friendlyanon/cmake-init)

## Github actions

## Motivation

1. We need a good way of running builds and tests on multiple platforms
2. We need to ensure  testing suite compliance without relying on the person to manually run it

GitHub Actions were made for this exact purpose, we can leverage them to run continuous integration tests.

### Useful links

- [Writing GitHub workflows](https://docs.github.com/en/actions/writing-workflows)
- [About continuous integration](https://docs.github.com/en/actions/about-github-actions/about-continuous-integration-with-github-actions)
- [About GitHub-hosted runners](https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners/about-github-hosted-runners)
