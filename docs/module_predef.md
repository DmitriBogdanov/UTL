[<img src ="images/badge_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](../LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_windows_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_ubuntu_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_macos_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::predef

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/predef.hpp)

**utl::predef** module contains macros and constant expressions for detecting compilation details.

It uses known implementation-defined macros to deduce compilation details and abstracts them away behind a unified API.

> [!Note]
> There exists a very similar [boost library](https://www.boost.org/doc/libs/1_55_0/libs/predef/doc/html/index.html) with the same name, it supports more exotic platforms, but has a different API and doesn't provide some of the things defined in this module.

## Definitions

```cpp
// Compiler detection
#define UTL_PREDEF_COMPILER_IS_MSVC // only one of these macros will be defined
#define UTL_PREDEF_COMPILER_IS_GCC
#define UTL_PREDEF_COMPILER_IS_CLANG
#define UTL_PREDEF_COMPILER_IS_LLVM
#define UTL_PREDEF_COMPILER_IS_ICC
#define UTL_PREDEF_COMPILER_IS_PGI
#define UTL_PREDEF_COMPILER_IS_IBMCPP
#define UTL_PREDEF_COMPILER_IS_NVCC
#define UTL_PREDEF_COMPILER_IS_UNKNOWN

constexpr std::string_view compiler_name;
constexpr std::string_view compiler_full_name;

// Platform detection
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64 // only one of these macros will be defined
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
#define UTL_PREDEF_PLATFORM_IS_CYGWIN
#define UTL_PREDEF_PLATFORM_IS_ANDROID
#define UTL_PREDEF_PLATFORM_IS_LINUX
#define UTL_PREDEF_PLATFORM_IS_UNIX
#define UTL_PREDEF_PLATFORM_IS_MACOS
#define UTL_PREDEF_PLATFORM_IS_UNKNOWN

constexpr std::string_view platform_name;

// Architecture detection
#define UTL_PREDEF_ARCHITECTURE_IS_X86_64 // only one of these macros will be defined
#define UTL_PREDEF_ARCHITECTURE_IS_X86_32
#define UTL_PREDEF_ARCHITECTURE_IS_ARM
#define UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN

constexpr std::string_view architecture_name;

// Language standard detection
#define UTL_PREDEF_STANDARD_IS_23_PLUS // multiple of these macros can be defined
#define UTL_PREDEF_STANDARD_IS_20_PLUS
#define UTL_PREDEF_STANDARD_IS_17_PLUS
#define UTL_PREDEF_STANDARD_IS_14_PLUS
#define UTL_PREDEF_STANDARD_IS_11_PLUS
#define UTL_DEFINE_STANDARD_IS_UNKNOWN

constexpr std::string_view standard_name;

// Compilation mode detection
#define UTL_PREDEF_MODE_IS_DEBUG

constexpr bool debug;

// Optimization macros
#define UTL_PREDEF_FORCE_INLINE
#define UTL_PREDEF_NO_INLINE
#define UTL_PREDEF_ASSUME

[[noreturn]] void unreachable();

// Other utils
std::string compilation_summary();
```

## Methods

### Compiler detection

> ```cpp
> // Compiler detection
> #define UTL_PREDEF_COMPILER_IS_MSVC
> #define UTL_PREDEF_COMPILER_IS_GCC
> #define UTL_PREDEF_COMPILER_IS_CLANG
> #define UTL_PREDEF_COMPILER_IS_LLVM
> #define UTL_PREDEF_COMPILER_IS_ICC
> #define UTL_PREDEF_COMPILER_IS_PGI
> #define UTL_PREDEF_COMPILER_IS_IBMCPP
> #define UTL_PREDEF_COMPILER_IS_NVCC
> #define UTL_PREDEF_COMPILER_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected compiler. If no other option is suitable, unknown is used as a fallback.

This is useful for compiler-specific conditional compilation.

> ```cpp
> constexpr std::string_view compiler_name;
> ```

`constexpr` string that evaluates to the abbreviated name of the detected compiler.

Possible values: `MSVC`, `GCC`, `clang`, `LLVM`, `ICC`, `PGI`, `IBMCPP`, `NVCC`, `<unknown>`.

> ```cpp
> constexpr std::string_view compiler_full_name;
> ```

`constexpr` string that evaluates to the full name of the detected compiler.

Possible values: `Microsoft Visual C++ Compiler`, `GNU C/C++ Compiler`, `Clang Compiler`, `LLVM Compiler`, `Inter C/C++ Compiler`, `Portland Group C/C++ Compiler`, `IBM XL C/C++ Compiler`, `Nvidia Cuda Compiler Driver`, `<unknown>`.

### Platform detection

> ```cpp
> // Platform detection
> #define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64
> #define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
> #define UTL_PREDEF_PLATFORM_IS_CYGWIN
> #define UTL_PREDEF_PLATFORM_IS_ANDROID
> #define UTL_PREDEF_PLATFORM_IS_LINUX
> #define UTL_PREDEF_PLATFORM_IS_UNIX
> #define UTL_PREDEF_PLATFORM_IS_MACOS
> #define UTL_PREDEF_PLATFORM_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected platform. If no other option is suitable, unknown is used as a fallback.

This is useful for platform-specific conditional compilation.

> ```cpp
> constexpr std::string_view platform_name;
> ```

`constexpr` string that evaluates to the name of the detected platform.

Possible values: `Windows64`, `Windows32`, `Windows (CYGWIN)`, `Android`, `Linux`, `Unix-like OS`, `MacOS`, `<unknown>`. 

### Architecture detection

> ```cpp
> #define UTL_PREDEF_ARCHITECTURE_IS_X86_64
> #define UTL_PREDEF_ARCHITECTURE_IS_X86_32
> #define UTL_PREDEF_ARCHITECTURE_IS_ARM
> #define UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected CPU architecture. If no other option is suitable, unknown is used as a fallback.

This is useful for architecture-specific conditional compilation.

> ```cpp
> constexpr std::string_view architecture_name;
> ```

`constexpr` string that evaluates to the name of the detected CPU architecture.

Possible values: `x86-64`, `x86-32`, `ARM`, `<unknown>`

### Language standard detection

> ```cpp
> #define UTL_PREDEF_STANDARD_IS_23_PLUS
> #define UTL_PREDEF_STANDARD_IS_20_PLUS
> #define UTL_PREDEF_STANDARD_IS_17_PLUS
> #define UTL_PREDEF_STANDARD_IS_14_PLUS
> #define UTL_PREDEF_STANDARD_IS_11_PLUS
> #define UTL_DEFINE_STANDARD_IS_UNKNOWN
> ```

**Multiple of these macros can be defined.** Macro that get defined correspond to the available C++ standard. If no other option is suitable, unknown is used as a fallback.

This is useful for conditional compilation based on available standard.

> ```cpp
> constexpr std::string_view standard_name;
> ```

`constexpr` string that evaluates to the name of the detected C++ standard. Possible values: `C++23`, `C++20`, `C++17`, `C++14`, `C++11`, `<unknown>`.

**Note:** Considering that this is a C++17 library, there should be no feasible way to get values below `C++17`, however they are still provided for the sake of implementation completeness, shall the source code be copied directly.

### Compilation mode detection

> ```cpp
> #define UTL_PREDEF_MODE_IS_DEBUG
> ```

Defined when compiling in debug mode. Works as an alias for `_DEBUG`, provided for the sake of feature completeness.

> ```cpp
> constexpr bool debug;
> ```

`constexpr` bool that evaluates to `true` when compiling in debug mode.

This is useful for `if constexpr` conditional compilation of debug code.

### Optimization macros

```cpp
#define UTL_PREDEF_FORCE_INLINE
```

Hints (`MSVC`) or forces (`GCC`, `clang` and `ICX`) function inlining using compiler built-ins.

**Note**: Compiles to nothing if there is no suitable compiler support.

```cpp
#define UTL_PREDEF_NO_INLINE
```

Disables function inlining using compiler built-ins.

**Note**: Compiles to regular `inline` if there is no suitable compiler support.

```cpp
#define UTL_PREDEF_ASSUME(condition)
```

Equivalent to [C++23 `[[assume(condition)]]`](https://en.cppreference.com/w/cpp/language/attributes/assume) that supports earlier standards using `MSVC` and `clang` built-ins.

Invokes undefined behavior if statement `condition` evaluates to false, which provides compiler with additional optimization opportunities since implementations may assume that undefined behavior can never happen and the statement always holds.

**Note 1**: Compiles to nothing if there is no suitable compiler support.

**Note 2:** In debug mode also works like an `assert()`.

```cpp
[[noreturn]] void unreachable();
```

Equivalent to [C++23 std::unreachable](https://en.cppreference.com/w/cpp/utility/unreachable) that supports earlier standards using `MSVC`, `clang` and `GCC` built-ins.

Compiler implementation may use this to optimize impossible code branches away.

**Note**: Even without compiler extensions UB is still raised by invoking an empty function as `[[noreturn]]`.

### Other utils

> ```cpp
> std::string compilation_summary();
> ```

Returns a string containing a detailed summary of compilation details based on the other functionality of this header. See [example](#compilation-summary).

## Examples

### Conditional compilation

[ [Run this code](https://godbolt.org/z/1TvKEqaYY) ]

```cpp
#if defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
std::cout << "Running Clang or GCC";
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
std::cout << "Running MSVC";
#else
std::cout << "Running some other compiler";
#endif
```

Output:
```
Running Clang or GCC
```

### Optimization macros

[ [Run this code](https://godbolt.org/z/jYzfTrEdE) ]

```cpp
enum class State { YES, NO };

UTL_PREDEF_FORCE_INLINE std::string to_string(State value) {
    switch (value) {
        case State::YES: return "YES";
        case State::NO : return "NO" ;
        default:         utl::predef::unreachable();
    }
}

// ...

assert( to_string(State::YES) == "YES" );
```

### Compilation summary

[ [Run this code](https://godbolt.org/z/PGYreYKor) ]

```cpp
std::cout << utl::predef::compilation_summary();
```

Output:
```
Compiler:          GNU C/C++ Compiler
Platform:          Linux
Architecture:      x86-64
L1 cache line (D): 64
L1 cache line (C): 64
Compiled in DEBUG: false
Compiled under OS: true
Compilation date:  Jul 19 2025 12:25:37
```