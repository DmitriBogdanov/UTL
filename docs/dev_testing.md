# Compilation

## GCC & clang

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

# CMake presets

## Problems

While seemingly a very clean general solution, [resets have several significant flaws complicating their usage in practice:

- Combinatorial explosion of configurations due to the lack of multiple `--preset` (see [issue](https://gitlab.kitware.com/cmake/cmake/-/issues/22538))
- Strong association between build / test presets and configure presets, which has no real reason to exist
- Inability comment `CMakePresets.json` makes it difficult to keep track of different build quirks

## Motivation

Despite all the issues, presets are still one of the few more-or-less sane ways of handling cross-platform building.

All platform-specific logic can be neatly enough isolated into `CMakePresets.json`.

# Github actions

