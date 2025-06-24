// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::predef
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_predef.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PREDEF)
#ifndef UTLHEADERGUARD_PREDEF
#define UTLHEADERGUARD_PREDEF

#define UTL_PREDEF_VERSION_MAJOR 1
#define UTL_PREDEF_VERSION_MINOR 0
#define UTL_PREDEF_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <string>      // string, to_string()
#include <string_view> // string_view
#include <utility>     // declval<>()

#ifdef __cpp_lib_hardware_interference_size
#include <new> // hardware_destructive_interference_size, hardware_constructive_interference_size
#endif

// ____________________ DEVELOPER DOCS ____________________

// Macros that provide a nicer way of querying some platform-specific stuff such as:
// compiler, platform, architecture, compilation info and etc.
//
// Boost Predef (https://www.boost.org/doc/libs/1_55_0/libs/predef/doc/html/index.html) provides
// a more complete package when it comes to supporting some esoteric platforms & compilers,
// but has a rather (in my opinion) ugly API.
//
// In addition utl::predef also provides some miscellaneous macros for automatic codegen, such as:
//    UTL_PREDEF_VA_ARGS_COUNT(args...)
//    UTL_PREDEF_IS_FUNCTION_DEFINED() - a nightmare of implementation, but it works
// some implementations may be rather sketchy due to trying to achieve things that weren't really
// meant to be achieved, but at the end of the day everything is standard-compliant.

// ____________________ IMPLEMENTATION ____________________

namespace utl::predef::impl {

// ================================
// --- Compiler Detection Macro ---
// ================================

#if defined(_MSC_VER)
#define UTL_PREDEF_COMPILER_IS_MSVC
#elif defined(__GNUC__) || defined(__GNUC_MINOR__) || defined(__GNUC_PATCHLEVEL__)
#define UTL_PREDEF_COMPILER_IS_GCC
#elif defined(__clang__) || defined(__clang_major__) || defined(__clang_minor__) || defined(__clang_patchlevel__)
#define UTL_PREDEF_COMPILER_IS_CLANG
#elif defined(__llvm__)
#define UTL_PREDEF_COMPILER_IS_LLVM
#elif defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)
#define UTL_PREDEF_COMPILER_IS_ICC
#elif defined(__PGI) || defined(__PGIC__) || defined(__PGIC_MINOR__) || defined(__PGIC_PATCHLEVEL__)
#define UTL_PREDEF_COMPILER_IS_PGI
#elif defined(__IBMCPP__) || defined(__xlC__) || defined(__xlc__)
#define UTL_PREDEF_COMPILER_IS_IBMCPP
#elif defined(__NVCC__) || defined(__CUDACC__)
#define UTL_PREDEF_COMPILER_IS_NVCC
#else
#define UTL_PREDEF_COMPILER_IS_UNKNOWN
#endif

constexpr std::string_view compiler_name =
#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
    "MSVC"
#elif defined(UTL_PREDEF_COMPILER_IS_GCC)
    "GCC"
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
    "clang"
#elif defined(UTL_PREDEF_COMPILER_IS_LLVM)
    "LLVM"
#elif defined(UTL_PREDEF_COMPILER_IS_ICC)
    "ICC"
#elif defined(UTL_PREDEF_COMPILER_IS_PGI)
    "PGI"
#elif defined(UTL_PREDEF_COMPILER_IS_IBMCPP)
    "IBMCPP"
#elif defined(UTL_PREDEF_COMPILER_IS_NVCC)
    "NVCC"
#else
    "<unknown>"
#endif
    ;

constexpr std::string_view compiler_full_name =
#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
    "Microsoft Visual C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_GCC)
    "GNU C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
    "Clang Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_LLVM)
    "LLVM Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_ICC)
    "Inter C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_PGI)
    "Portland Group C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_IBMCPP)
    "IBM XL C/C++ Compiler"
#elif defined(UTL_PREDEF_COMPILER_IS_NVCC)
    "Nvidia Cuda Compiler Driver"
#else
    "<unknown>"
#endif
    ;

// ================================
// --- Platform Detection Macro ---
// ================================

#if defined(_WIN64) // _WIN64 implies _WIN32 so it should be first
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64
#elif defined(_WIN32)
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
#elif defined(__CYGWIN__) && !defined(_WIN32) // Cygwin POSIX under Microsoft Window
#define UTL_PREDEF_PLATFORM_IS_CYGWIN
#elif defined(__ANDROID__) // __ANDROID__ implies __linux__ so it should be first
#define UTL_PREDEF_PLATFORM_IS_ANDROID
#elif defined(linux) || defined(__linux__) || defined(__linux)
#define UTL_PREDEF_PLATFORM_IS_LINUX
#elif defined(unix) || defined(__unix__) || defined(__unix)
#define UTL_PREDEF_PLATFORM_IS_UNIX
#elif defined(__APPLE__) && defined(__MACH__)
#define UTL_PREDEF_PLATFORM_IS_MACOS
#else
#define UTL_PREDEF_PLATFORM_IS_UNKNOWN
#endif

constexpr std::string_view platform_name =
#if defined(UTL_PREDEF_PLATFORM_IS_WINDOWS_X64)
    "Windows64"
#elif defined(UTL_PREDEF_PLATFORM_IS_WINDOWS_X32)
    "Windows32"
#elif defined(UTL_PREDEF_PLATFORM_IS_CYGWIN)
    "Windows (CYGWIN)"
#elif defined(UTL_PREDEF_PLATFORM_IS_ANDROID)
    "Android"
#elif defined(UTL_PREDEF_PLATFORM_IS_LINUX)
    "Linux"
#elif defined(UTL_PREDEF_PLATFORM_IS_UNIX)
    "Unix-like OS"
#elif defined(UTL_PREDEF_PLATFORM_IS_MACOS)
    "MacOS" // Apple OSX and iOS (Darwin)
#else
    "<unknown>"
#endif
    ;


// ====================================
// --- Architecture Detection Macro ---
// ====================================

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64)
#define UTL_PREDEF_ARCHITECTURE_IS_X86_64
#elif defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) ||             \
    defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) ||             \
    defined(__INTEL__) || defined(__I86__) || defined(_M_IX86) || defined(__i686__) || defined(__i586__) ||            \
    defined(__i486__) || defined(__i386__)
#define UTL_PREDEF_ARCHITECTURE_IS_X86_32
#elif defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) ||          \
    defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB)
#define UTL_PREDEF_ARCHITECTURE_IS_ARM
#else
#define UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN
#endif

constexpr std::string_view architecture_name =
#if defined(UTL_PREDEF_ARCHITECTURE_IS_X86_64)
    "x86-64"
#elif defined(UTL_PREDEF_ARCHITECTURE_IS_X86_32)
    "x86-32"
#elif defined(UTL_PREDEF_ARCHITECTURE_IS_ARM)
    "ARM"
#else
    "<unknown>"
#endif
    ;

// =========================================
// --- Language Standard Detection Macro ---
// =========================================

#if defined(UTL_PREDEF_COMPILER_IS_MSVC)
#define UTL_PREDEF_CPP_VERSION _MSVC_LANG
#else
#define UTL_PREDEF_CPP_VERSION __cplusplus
#endif
// Note 1:
// MSVC '__cplusplus' is defined, but stuck at '199711L'. It uses '_MSVC_LANG' instead.
//
// Note 2:
// '__cplusplus' is defined by the standard, it's only Microsoft who think standards are for other people.
//
// Note 3:
// MSVC has a flag '/Zc:__cplusplus' that enables standard behaviour for '__cplusplus'

#if (UTL_PREDEF_CPP_VERSION >= 202302L)
#define UTL_PREDEF_STANDARD_IS_23_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 202002L)
#define UTL_PREDEF_STANDARD_IS_20_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201703L)
#define UTL_PREDEF_STANDARD_IS_17_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201402L)
#define UTL_PREDEF_STANDARD_IS_14_PLUS
#elif (UTL_PREDEF_CPP_VERSION >= 201103L)
#define UTL_PREDEF_STANDARD_IS_11_PLUS
#else // everything below C++11 has the same value of '199711L'
#define UTL_PREDEF_STANDARD_IS_UNKNOWN
#endif
// Note:
// There should be no feasible way to fall below the 'UTL_PREDEF_STANDARD_IS_17_PLUS' since this library itself
// requires C++17 to compile, but might as well have a complete implementation for future reference.

constexpr std::string_view standard_name =
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
    "C++23"
#elif defined(UTL_PREDEF_STANDARD_IS_20_PLUS)
    "C++20"
#elif defined(UTL_PREDEF_STANDARD_IS_17_PLUS)
    "C++17"
#elif defined(UTL_PREDEF_STANDARD_IS_14_PLUS)
    "C++14"
#elif defined(UTL_PREDEF_STANDARD_IS_11_PLUS)
    "C++11"
#else
    "<unknown>"
#endif
    ;

// ========================================
// --- Compilation Mode Detection Macro ---
// ========================================

#if defined(_DEBUG)
#define UTL_PREDEF_MODE_IS_DEBUG
#endif

constexpr bool debug =
#if defined(UTL_PREDEF_MODE_IS_DEBUG)
    true
#else
    false
#endif
    ;

// ===========================
// --- Optimization macros ---
// ===========================

// Note:
// These are mainly valuable as a reference implementation for portable optimization built-ins,
// which is why they are made to independent of other macros in this module.

// Force inline
// (requires regular 'inline' after the macro)
#if defined(_MSC_VER)
#define UTL_PREDEF_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define UTL_PREDEF_FORCE_INLINE __attribute__((always_inline))
#else
#define UTL_PREDEF_FORCE_INLINE
#endif

// Force noinline
#if defined(_MSC_VER)
#define UTL_PREDEF_FORCE_NOINLINE __declspec((noinline))
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define UTL_PREDEF_FORCE_NOINLINE __attribute__((noinline))
#endif

// Branch prediction hints
// (legacy, use '[[likely]]', '[[unlikely]] in C++20 and on)
#if defined(__GNUC__) || defined(__clang__)
#define UTL_PREDEF_LEGACY_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define UTL_PREDEF_LEGACY_LIKELY(x) (x)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define UTL_PREDEF_LEGACY_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define UTL_PREDEF_LEGACY_UNLIKELY(x) (x)
#endif

// Assume condition
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
#define UTL_PREDEF_ASSUME(...) [[assume(__VA_ARGS__))]]
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
#define UTL_PREDEF_ASSUME(...) __assume(__VA_ARGS__)
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
#define UTL_PREDEF_ASSUME(...) __builtin_assume(__VA_ARGS__)
#else // no equivalent GCC built-in
#define UTL_PREDEF_ASSUME(...) __VA_ARGS__
#endif

[[noreturn]] inline void unreachable() {
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
    std::unreachable();
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
    __assume(false);
#elif defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
    __builtin_unreachable();
#endif
}

// ===================
// --- Other Utils ---
// ===================

[[nodiscard]] inline std::string compilation_summary() {
    std::string buffer;

    buffer += "Compiler:          ";
    buffer += compiler_full_name;
    buffer += '\n';

    buffer += "Platform:          ";
    buffer += platform_name;
    buffer += '\n';

    buffer += "Architecture:      ";
    buffer += architecture_name;
    buffer += '\n';

#ifdef __cpp_lib_hardware_interference_size
    buffer += "L1 cache line (D):  ";
    buffer += std::to_string(std::hardware_destructive_interference_size);
    buffer += '\n';

    buffer += "L1 cache line (C):  ";
    buffer += std::to_string(std::hardware_constructive_interference_size);
    buffer += '\n';
#endif // not (currently) implemented in GCC / clang despite being a C++17 feature

    buffer += "Compiled in DEBUG: ";
    buffer += debug ? "true" : "false";
    buffer += '\n';

    buffer += "Compiled under OS: ";
    buffer += __STDC_HOSTED__ ? "true" : "false";
    buffer += '\n';

    buffer += "Compilation date:  ";
    buffer += __DATE__;
    buffer += ' ';
    buffer += __TIME__;
    buffer += '\n';

    return buffer;
}

// ===================
// --- Macro Utils ---
// ===================

// --- Size of __VA_ARGS__ in variadic macros ---
// ----------------------------------------------

#define utl_predef_expand_va_args(x_) x_ // a fix for MSVC bug not expanding __VA_ARGS__ properly

#define utl_predef_va_args_count_impl(x01_, x02_, x03_, x04_, x05_, x06_, x07_, x08_, x09_, x10_, x11_, x12_, x13_,    \
                                      x14_, x15_, x16_, x17_, x18_, x19_, x20_, x21_, x22_, x23_, x24_, x25_, x26_,    \
                                      x27_, x28_, x29_, x30_, x31_, x32_, x33_, x34_, x35_, x36_, x37_, x38_, x39_,    \
                                      x40_, x41_, x42_, x43_, x44_, x45_, x46_, x47_, x48_, x49_, N_, ...)             \
    N_

#define UTL_PREDEF_VA_ARGS_COUNT(...)                                                                                  \
    utl_predef_expand_va_args(utl_predef_va_args_count_impl(                                                           \
        __VA_ARGS__, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26,   \
        25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// --- Map function macro to __VA_ARGS__ ---
// -----------------------------------------

#define utl_predef_map_eval_0(...) __VA_ARGS__
#define utl_predef_map_eval_1(...) utl_predef_map_eval_0(utl_predef_map_eval_0(utl_predef_map_eval_0(__VA_ARGS__)))
#define utl_predef_map_eval_2(...) utl_predef_map_eval_1(utl_predef_map_eval_1(utl_predef_map_eval_1(__VA_ARGS__)))
#define utl_predef_map_eval_3(...) utl_predef_map_eval_2(utl_predef_map_eval_2(utl_predef_map_eval_2(__VA_ARGS__)))
#define utl_predef_map_eval_4(...) utl_predef_map_eval_3(utl_predef_map_eval_3(utl_predef_map_eval_3(__VA_ARGS__)))
#define utl_predef_map_eval(...) utl_predef_map_eval_4(utl_predef_map_eval_4(utl_predef_map_eval_4(__VA_ARGS__)))

#define utl_predef_map_end(...)
#define utl_predef_map_out
#define utl_predef_map_comma ,

#define utl_predef_map_get_end_2() 0, utl_predef_map_end
#define utl_predef_map_get_end_1(...) utl_predef_map_get_end2
#define utl_predef_map_get_end(...) utl_predef_map_get_end_1
#define utl_predef_map_next_0(test, next, ...) next utl_predef_map_out
#define utl_predef_map_next_1(test, next) utl_predef_map_next_0(test, next, 0)
#define utl_predef_map_next(test, next) utl_predef_map_next_1(utl_predef_map_get_end test, next)

#define utl_predef_map_0(f, x, peek, ...) f(x) utl_predef_map_next(peek, utl_predef_map_1)(f, peek, __VA_ARGS__)
#define utl_predef_map_1(f, x, peek, ...) f(x) utl_predef_map_next(peek, utl_predef_map_0)(f, peek, __VA_ARGS__)

#define utl_predef_map_list_next_1(test, next) utl_predef_map_next_0(test, utl_predef_map_comma next, 0)
#define utl_predef_map_list_next(test, next) utl_predef_map_list_next_1(utl_predef_map_get_end test, next)

#define utl_predef_map_list_0(f, x, peek, ...)                                                                         \
    f(x) utl_predef_map_list_next(peek, utl_predef_map_list_1)(f, peek, __VA_ARGS__)
#define utl_predef_map_list_1(f, x, peek, ...)                                                                         \
    f(x) utl_predef_map_list_next(peek, utl_predef_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro `f` to each of the remaining parameters.
#define UTL_PREDEF_MAP(f, ...) utl_predef_map_eval(utl_predef_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro `f` to each of the remaining parameters and
// inserts commas between the results.
#define UTL_PREDEF_MAP_LIST(f, ...)                                                                                    \
    utl_predef_map_eval(utl_predef_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// ===============
// --- Codegen ---
// ===============

// --- Type trait generation ---
// -----------------------------

// This macro generates a type trait 'trait_name_' that returns 'true' for any 'T'
// such that 'T'-dependent expression passed into '...' compiles.
//
// Also generates as helper-constant 'trait_name_##_v` like the one all standard traits provide.
//
// Also generates as shortcut for 'enable_if' based on that trait.
//
// This macro saves MASSIVE amount of boilerplate in some cases, making for a much more expressive "trait definitions".

#define UTL_PREDEF_TYPE_TRAIT(trait_name_, ...)                                                                        \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;                                                            \
                                                                                                                       \
    template <class T>                                                                                                 \
    using trait_name_##_enable_if = std::enable_if_t<trait_name_<T>::value, bool>

// Shortcuts for different types of requirements
#define UTL_PREDEF_TYPE_TRAIT_HAS_BINARY_OP(trait_name_, op_)                                                          \
    UTL_PREDEF_TYPE_TRAIT(trait_name_, std::declval<std::decay_t<T>>() op_ std::declval<std::decay_t<T>>())

#define UTL_PREDEF_TYPE_TRAIT_HAS_ASSIGNMENT_OP(trait_name_, op_)                                                      \
    UTL_PREDEF_TYPE_TRAIT(trait_name_, std::declval<std::decay_t<T>&>() op_ std::declval<std::decay_t<T>>())
// for operators like '+=' lhs should be a reference

#define UTL_PREDEF_TYPE_TRAIT_HAS_UNARY_OP(trait_name_, op_)                                                           \
    UTL_PREDEF_TYPE_TRAIT(trait_name_, op_ std::declval<std::decay_t<T>>())

#define UTL_PREDEF_TYPE_TRAIT_HAS_MEMBER(trait_name_, member_)                                                         \
    UTL_PREDEF_TYPE_TRAIT(trait_name_, std::declval<std::decay_t<T>>().member_)

#define UTL_PREDEF_TYPE_TRAIT_HAS_MEMBER_TYPE(trait_name_, member_)                                                    \
    UTL_PREDEF_TYPE_TRAIT(trait_name_, std::declval<typename std::decay_t<T>::member_>())

// --- Arcane junk with no purpose ---
// -----------------------------------

#define UTL_PREDEF_IS_FUNCTION_DEFINED(function_name_, return_type_, ...)                                              \
    template <class ReturnType, class... ArgTypes>                                                                     \
    class utl_is_function_defined_impl_##function_name_ {                                                              \
    private:                                                                                                           \
        typedef char no[sizeof(ReturnType) + 1];                                                                       \
                                                                                                                       \
        template <class... C>                                                                                          \
        static auto test(C... arg) -> decltype(function_name_(arg...));                                                \
                                                                                                                       \
        template <class... C>                                                                                          \
        static no& test(...);                                                                                          \
                                                                                                                       \
    public:                                                                                                            \
        enum { value = (sizeof(test<ArgTypes...>(std::declval<ArgTypes>()...)) == sizeof(ReturnType)) };               \
    };                                                                                                                 \
                                                                                                                       \
    using is_function_defined_##function_name_ =                                                                       \
        utl_is_function_defined_impl_##function_name_<return_type_, __VA_ARGS__>;
// TASK:
// We need to detect at compile time if function FUNC(ARGS...) exists.
// FUNC identifier isn't guaranteed to be declared.
//
// Ideal method would look like UTL_FUNC_EXISTS(FUNC, RETURN_TYPE, ARG_TYPES...) -> true/false
// This does not seem to be possible, we have to declare integral constant instead, see explanation below.
//
// WHY IS IT SO HARD:
// (1) Can this be done through preprocessor macros?
// No, preprocessor has no way to tell whether C++ identifier is defined or not.
//
// (2) Is there a compiler-specific way to do it?
// Doesn't seem to be the case.
//
// (3) Why not use some sort of template with FUNC as a parameter?
// Essentially we have to evaluate undeclared identifier, while compiler exits with error upon
// encountering anything undeclared. The only way to detect whether undeclared identifier exists
// or not seems to be through SFINAE.
//
// IMPLEMENTATION COMMENTS:
// We declare integral constant class with 2 functions 'test()', first one takes priority during overload
// resolution and compiles if FUNC(ARGS...) is defined, otherwise it's {Substitution Failure} which is
// {Is Not An Error} and second function compiles.
//
// To resolve which overload of 'test()' was selected we check the sizeof() return type, 2nd overload
// has a return type 'char[sizeof(ReturnType) + 1]' so it's always different from 1st overload.
// Resolution result (true/false) gets stored to '::value'.
//
// Note that we can't pass 'ReturnType' and 'ArgTypes' directly through '__VA_ARGS__' because
// to call function 'test(ARGS...)' in general case we have to 'std::declval<>()' all 'ARGS...'.
// To do so we can use variadic template syntax and then just forward '__VA_ARGS__' to the template
// through 'using is_function_present = is_function_present_impl<ReturnType, __VA_ARGS__>'.
//
// ALTERNATIVES: Perhaps some sort of tricky inline SFINAE can be done through C++14 generic lambdas.
//
// NOTE 1: Some versions of 'clangd' give a 'bugprone-sizeof-expression' warning for sizeof(*A),
// this is a false alarm.
//
// NOTE 2: Frankly, the usefulness of this is rather dubious since constructs like
//     if constexpr (is_function_defined_windown_specific) { <call the windows-specific function> }
//     else { <call the linux-specific function> }
// are still illegal due to 'if constexpr' requiting both branches to have defined identifiers,
// but since this arcane concept is already implemented why not keep it.

} // namespace utl::predef::impl

// ______________________ PUBLIC API ______________________

namespace utl::predef {

// macro -> UTL_PREDEF_COMPILER_IS_...
using impl::compiler_name;
using impl::compiler_full_name;

// macro -> UTL_PREDEF_PLATFORM_IS_...
using impl::platform_name;

// macro -> UTL_PREDEF_ARCHITECTURE_IS_...
using impl::architecture_name;

// macro -> UTL_PREDEF_STANDARD_IS_...
using impl::standard_name;

// macro -> UTL_PREDEF_MODE_IS_DEBUG
using impl::debug;

// macro -> UTL_PREDEF_FORCE_INLINE
// macro -> UTL_PREDEF_ASSUME
using impl::unreachable;

using impl::compilation_summary;
// macro -> UTL_PREDEF_VA_ARGS_COUNT
// macro -> UTL_PREDEF_IS_FUNCTION_DEFINED_TRAIT

} // namespace utl::predef

#endif
#endif // module utl::predef
