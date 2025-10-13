// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::assertion
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_assertion.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_ASSERTION)

#ifndef utl_assertion_headerguard
#define utl_assertion_headerguard

#define UTL_ASSERTION_VERSION_MAJOR 1
#define UTL_ASSERTION_VERSION_MINOR 0
#define UTL_ASSERTION_VERSION_PATCH 4

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <cstdlib>     // abort()
#include <functional>  // function<>
#include <iostream>    // cerr
#include <mutex>       // mutex, scoped_lock
#include <sstream>     // ostringstream
#include <string>      // string
#include <string_view> // string_view
#include <utility>     // forward(), move()

// ____________________ DEVELOPER DOCS ____________________

// The key to nice assertion messages is expression decomposition, we want to print something like:
//    | Error: assertion {x + y < z} evaluated to {4 < 3}
// instead of a regular message with no diagnostics.
//
// Lets consider some expression, for example 'x + y > z * 4'. Ideally, we want to non-intrusively decompose this
// expression into its individual parts so we can print the values of all variables ('x', 'y' and 'z').
//
// In a general case this is not possible in C++, however if we restrict expressions to the form:
//    | {lhs} {comparison} {rhs}
// where 'lhs' and 'rhs' are some values evaluated before the 'comparison', then we can use a certain trick
// based on operator precedence to extract and print 'lhs' / 'rhs' / 'comparison'. For the example above we have:
//    | x + y <= z * 4
//    | ^^^^^    ^^^^^
//    | {lhs}    {rhs}
//
// Now let's declare:
//    - 'Info'          object which carries assertion info (callsite, message, etc.)
//    - 'UnaryCapture'  object which carries 'info' + 'lhs'
//    - 'BinaryCapture' object which carries 'info' + 'lhs' + 'rhs'
// and write:
//    | info < x + y <= z * 4
// which is a non-intrusive expression which will look like
//    | info < {expr}
// when evaluated in a general macro.
//
// We can declare custom 'operator<()' that wraps 'info' + 'lhs' into an 'UnaryCapture', and custom set of comparisons
// turning 'UnaryCapture' + 'rhs' into a 'BinaryCapture'. Due to the operator precedence 'lhs' / 'rhs' will be evaluated
// before the comparisons so we can always capture them. This might produce some warnings, but we can silence them.
//
// After this the captured expression can be forwarded to a relatively standard assertion handler, which will use
// this decomposition for pretty printing and more debug info. Performance-wise the cost should be minimal since
// this is effectively the same thing as expression templates, but simpler.

// ____________________ IMPLEMENTATION ____________________

namespace utl::assertion::impl {

// ===============
// --- Utility ---
// ===============

template <class T>
[[nodiscard]] std::string stringify(const T& value) {
    return (std::ostringstream{} << value).str(); // in C++20 can be done much faster with std::format
}

template <class... Args>
void append_fold(std::string& str, const Args&... args) {
    ((str += args), ...);
} // faster than 'std::ostringstream'

[[nodiscard]] inline std::string_view trim_to_filename(std::string_view path) {
    const std::size_t last_slash = path.find_last_of("/\\");
    return path.substr(last_slash + 1);
}

namespace colors {

constexpr std::string_view cyan         = "\033[36m";
constexpr std::string_view bold_red     = "\033[31;1m";
constexpr std::string_view bold_blue    = "\033[34;1m";
constexpr std::string_view bold_magenta = "\033[35;1m";
constexpr std::string_view reset        = "\033[0m";

} // namespace colors

// SFINAE to restrict assertion captures to printable types. It doesn't affect functionality
// since non-printable types would cause compile error regardless, but we can use it to
// improve LSP highlighting and error message by failing the instantiation early
template <class T, class = void>
struct is_printable : std::false_type {};

template <class T>
struct is_printable<T, std::void_t<decltype(std::declval<std::ostringstream>() << std::declval<T>())>>
    : std::true_type {};

// =============================
// --- Decomposed operations ---
// =============================

enum class Operation : std::size_t {
    EQ  = 0, // ==
    NEQ = 1, // !=
    LEQ = 2, // <=
    GEQ = 3, // >=
    L   = 4, // <
    G   = 5  // >
};

constexpr std::array<const char*, 6> op_names = {" == ", " != ", " <= ", " >= ", " < ", " > "};

// ======================
// --- Assertion info ---
// ======================

// Lightweight struct that captures the assertion context internally
struct Info {
    const char* file;
    int         line;
    const char* func;

    const char* expression;
    const char* context;
};

// Once we hit a slow failure path we can convert internal info to a nicer format for public API
class FailureInfo {
    std::string evaluated_string;
    // since evaluated string is constructed at runtime we have to store it here,
    // while exposing string_view in a public API for the sake of uniformity

public:
    FailureInfo(const Info& info, std::string evaluated_string)
        : evaluated_string(std::move(evaluated_string)), file(info.file), line(static_cast<std::size_t>(info.line)),
          func(info.func), expression(info.expression), evaluated(this->evaluated_string), context(info.context) {}

    std::string_view file;
    std::size_t      line;
    std::string_view func;

    std::string_view expression;
    std::string_view evaluated;
    std::string_view context;

    [[nodiscard]] std::string to_string(bool color = false) const {
        constexpr auto indent_single = "    ";
        constexpr auto indent_double = "        ";

        const auto color_assert    = color ? colors::bold_red : "";
        const auto color_file      = color ? colors::bold_blue : "";
        const auto color_func      = color ? colors::bold_magenta : "";
        const auto color_text      = color ? colors::bold_red : "";
        const auto color_expr      = color ? colors::cyan : "";
        const auto color_evaluated = color ? colors::cyan : "";
        const auto color_context   = color ? colors::cyan : "";
        const auto color_reset     = color ? colors::reset : "";

        std::string res;

#ifdef UTL_ASSERTION_ENABLE_FULL_PATHS
        const auto displayed_file = this->file;
#else
        const auto displayed_file = trim_to_filename(this->file);
#endif

        // "Assertion failed at {file}:{line}: {func}"
        append_fold(res, color_assert, "Assertion failed at ", color_reset);
        append_fold(res, color_file, displayed_file, ":", std::to_string(this->line), color_reset);
        append_fold(res, color_assert, ": ", color_reset, color_func, this->func, color_reset, '\n');
        // "Where condition: {expr}"
        append_fold(res, indent_single, color_text, "Where condition:", color_reset, color_reset, '\n');
        append_fold(res, indent_double, color_expr, this->expression, color_reset, '\n');
        // "Evaluated to: {eval}"
        append_fold(res, indent_single, color_text, "Evaluated to:", color_reset, '\n');
        append_fold(res, indent_double, color_evaluated, this->evaluated, color_reset, '\n');
        // "Context: {message}"
        append_fold(res, indent_single, color_text, "Context:", color_reset, '\n');
        append_fold(res, color_context, indent_double, this->context, color_reset, '\n');

        // Note: Trimming path to

        return res;
    }
};

// =======================
// --- Failure handler ---
// =======================

inline void standard_handler(const FailureInfo& info) {
    std::cerr << info.to_string(true) << std::endl;
    std::abort();
}

class GlobalHandler {
    std::function<void(const FailureInfo&)> handler = standard_handler;
    std::mutex                              mutex;
    // regular 'assert()' doesn't need thread safety since it always aborts, in a general case
    // with custom handlers however thread safety on failure should be provided

public:
    static GlobalHandler& instance() {
        static GlobalHandler handler;
        return handler;
    }

    void set(std::function<void(const FailureInfo&)> new_handler) {
        const std::scoped_lock lock(this->mutex);

        this->handler = std::move(new_handler);
    }

    void invoke(const FailureInfo& info) {
        const std::scoped_lock lock(this->mutex);

        this->handler(info);
    }
};

inline void set_handler(std::function<void(const FailureInfo&)> new_handler) {
    GlobalHandler::instance().set(std::move(new_handler));
}

// =====================
// --- Unary capture ---
// =====================

template <class T>
struct UnaryCapture {
    static_assert(is_printable<T>::value,
                  "Decomposed expression values should be printable with 'std::ostream::operator<<()'.");

    const Info& info;

    T value;

    FailureInfo get_failure_info() const {
        if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            return {this->info, "false"}; // makes boolean case look nicer
        } else if constexpr (std::is_pointer_v<std::decay_t<T>>) {
            return {this->info, "nullptr (converts to false)"}; // makes pointer case look nicer
        } else {
            std::string evaluated = stringify(this->value) + " (converts to false)";
            return {this->info, std::move(evaluated)};
        }
    }
};

template <class T>
UnaryCapture<T> operator<(const Info& info, T&& value) noexcept(noexcept(T(std::forward<T>(value)))) {
    return {info, std::forward<T>(value)};
} // Note: Successful assertions should be 'noexcept' if possible, this also applies to the binary case

template <class T>
void handle_capture(UnaryCapture<T>&& capture) {
    if (static_cast<bool>(capture.value)) return; // some compilers might complain without explicit casting
    GlobalHandler::instance().invoke(capture.get_failure_info());
}

// ======================
// --- Binary capture ---
// ======================

template <class T, class U, Operation Op>
struct BinaryCapture {
    static_assert(is_printable<T>::value && is_printable<U>::value,
                  "Decomposed expression values should be printable with 'std::ostream::operator<<()'.");

    const Info& info;

    T lhs;
    U rhs;

    FailureInfo get_failure_info() const {
        constexpr std::size_t op_index = static_cast<std::size_t>(Op);

        std::string evaluated = stringify(this->lhs) + op_names[op_index] + stringify(this->rhs);
        return {this->info, std::move(evaluated)};
    }
};

// Macro to avoid 6x code repetition
#define utl_assertion_define_binary_capture_op(op_enum_, op_)                                                          \
    template <class T, class U>                                                                                        \
    BinaryCapture<T, U, op_enum_> operator op_(UnaryCapture<T>&& lhs, U&& rhs) noexcept(                               \
        std::is_nothrow_move_constructible_v<T> && noexcept(U(std::forward<U>(rhs)))) {                                \
                                                                                                                       \
        return {lhs.info, std::move(lhs).value, std::forward<U>(rhs)};                                                 \
    }                                                                                                                  \
                                                                                                                       \
    template <class T, class U>                                                                                        \
    void handle_capture(BinaryCapture<T, U, op_enum_>&& capture) {                                                     \
        if (capture.lhs op_ capture.rhs) return;                                                                       \
        GlobalHandler::instance().invoke(capture.get_failure_info());                                                  \
    }                                                                                                                  \
                                                                                                                       \
    static_assert(true)

utl_assertion_define_binary_capture_op(Operation::EQ, ==);
utl_assertion_define_binary_capture_op(Operation::NEQ, !=);
utl_assertion_define_binary_capture_op(Operation::LEQ, <=);
utl_assertion_define_binary_capture_op(Operation::GEQ, >=);
utl_assertion_define_binary_capture_op(Operation::L, <);
utl_assertion_define_binary_capture_op(Operation::G, >);

#undef utl_assertion_define_binary_capture_op

// =======================
// --- Pretty function ---
// =======================

// Makes assertion diagnostics a bit nicer
#if defined(__clang__) || defined(__GNUC__)
#define utl_check_pretty_function __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define utl_check_pretty_function __FUNCSIG__
#else
#define utl_check_pretty_function __func__
#endif

// ======================================
// --- Macros with optional arguments ---
// ======================================

// Standard-compliant macro to achieve macro overloading. This could be achieved easier with a common
// GCC/clang/MSVC extension that removes a trailing '__VA_ARGS__' comma, or with C++20 '__VA_OPT__'.
// Here we have a pedantic implementation for up to 3 args based on the notes of Jason Deng & Kuukunen,
// see: https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros

#define utl_assertion_func_chooser(_f0, _f1, _f2, _f3, ...) _f3
#define utl_assertion_func_composer(enclosed_args_) utl_assertion_func_chooser enclosed_args_
#define utl_assertion_choose_from_arg_count(F, ...) utl_assertion_func_composer((__VA_ARGS__, F##_3, F##_2, F##_1, ))
#define utl_assertion_narg_expander(f_) , , , f_##_0
#define utl_assertion_macro_chooser(f_, ...)                                                                           \
    utl_assertion_choose_from_arg_count(f_, utl_assertion_narg_expander __VA_ARGS__(f_))

#define utl_assertion_overloaded_macro(f_, ...) utl_assertion_macro_chooser(f_, __VA_ARGS__)(__VA_ARGS__)

// ===================================
// --- Assert macro implementation ---
// ===================================

#define utl_assertion_impl_2(expr_, context_)                                                                          \
    utl::assertion::impl::handle_capture(                                                                              \
        utl::assertion::impl::Info{__FILE__, __LINE__, utl_check_pretty_function, #expr_, context_} < expr_)

#define utl_assertion_impl_1(expr_) utl_assertion_impl_2(expr_, "<no context provided>")

#define utl_assertion_impl_0() static_assert(false, "Cannot invoke an assertion with no arguments.")

} // namespace utl::assertion::impl

// ______________________ PUBLIC API ______________________

// =======================
// --- Assertion macro ---
// =======================

// Disable false positive warning about operator precedence,
// while error-prone for the regular use cases 'expr < lhs < rhs'
// in this context is exactly what we want since this is the only way
// of implementing the desired expression decomposition.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wparentheses"
#pragma clang diagnostic push
#elif __GNUC__
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic push
#endif

#if !defined(NDEBUG) || defined(UTL_ASSERTION_ENABLE_IN_RELEASE)
#define UTL_ASSERTION(...) utl_assertion_overloaded_macro(utl_assertion_impl, __VA_ARGS__)
#else
#define UTL_ASSERTION(...) static_assert(true)
#endif

// Turn the warnings back on
#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

// =========================
// --- Optional shortcut ---
// =========================

#ifdef UTL_ASSERTION_ENABLE_SHORTCUT
#define ASSERT(...) UTL_ASSERTION(__VA_ARGS__)
#endif

// =====================
// --- Non-macro API ---
// =====================

namespace utl::assertion {

using impl::FailureInfo;

using impl::set_handler;

} // namespace utl::assertion

#endif
#endif // module utl::assertion
