[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::predef

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/predef.hpp)

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:20,endLineNumber:3,positionColumn:1,positionLineNumber:3,selectionStartColumn:20,selectionStartLineNumber:3,startColumn:1,startLineNumber:3),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/predef.hpp%3E%0A%0A%23include+%3Ciostream%3E%0A%0Aint+main()+%7B%0A++++%23if+defined(UTL_PREDEF_COMPILER_IS_GCC)+%7C%7C+defined(UTL_PREDEF_COMPILER_IS_CLANG)%0A++++std::cout+%3C%3C+%22Running+Clang+or+GCC%22%3B%0A++++%23elif+defined(UTL_PREDEF_COMPILER_IS_MSVC)%0A++++std::cout+%3C%3C+%22Running+MSVC%22%3B%0A++++%23else%0A++++std::cout+%3C%3C+%22Running+some+other+compiler%22%3B%0A++++%23endif%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:4,positionColumn:1,positionLineNumber:4,selectionStartColumn:1,selectionStartLineNumber:4,startColumn:1,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/predef.hpp%3E%0A%0Aenum+class+State+%7B+YES,+NO+%7D%3B%0A%0AUTL_PREDEF_FORCE_INLINE+//+tells+compiler+to+always+inline+the+function%0Astd::string+to_string(State+value)+%7B%0A++++switch+(value)+%7B%0A++++++++case+State::YES:+return+%22YES%22%3B%0A++++++++case+State::NO+:+return+%22NO%22+%3B%0A++++++++default:+utl::predef::unreachable()%3B%0A++++++++//+tells+compiler+that+!'default!'+case+will+never+be+executed%0A++++%7D%0A%7D%0A%0Aint+main()+%7B%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
enum class State { YES, NO };

UTL_PREDEF_FORCE_INLINE // tells compiler to always inline the function
std::string to_string(State value) {
    switch (value) {
        case State::YES: return "YES";
        case State::NO : return "NO" ;
        default: utl::predef::unreachable();
        // tells compiler that 'default' case will never be executed
    }
}
```

### Compilation summary

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:4,positionColumn:1,positionLineNumber:4,selectionStartColumn:1,selectionStartLineNumber:4,startColumn:1,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/predef.hpp%3E%0A%0A%23include+%3Ciostream%3E%0A%0Aint+main()+%7B%0A++++std::cout+%3C%3C+utl::predef::compilation_summary()%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
std::cout << utl::predef::compilation_summary();
```

Output:
```
Compiler:          GNU C/C++ Compiler
Platform:          Linux
Architecture:      x86-64
Compiled in DEBUG: false
Compiled under OS: true
Compilation date:  Dec  1 2024 03:47:20
```