// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::describe_struct
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_describe_struct.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_DESCRIBE_STRUCT)

#ifndef utl_describe_struct_headerguard
#define utl_describe_struct_headerguard

#define UTL_DESCRIBE_STRUCT_VERSION_MAJOR 2
#define UTL_DESCRIBE_STRUCT_VERSION_MINOR 0
#define UTL_DESCRIBE_STRUCT_VERSION_PATCH 3

// _______________________ INCLUDES _______________________

#include <array>       // IWYU pragma: keep (used in a macro) | array<>, size_t
#include <string_view> // IWYU pragma: keep (used in a macro) | string_view
#include <tuple>       // tuple<>, tuple_size<>, get<>(), make_tuple()
#include <type_traits> // remove_cvref<>
#include <utility>     // forward<>(), pair<>

// ____________________ DEVELOPER DOCS ____________________

// Reflection mechanism is based entirely around the map macro and a single struct with partial specialization for the
// reflected enum. Map macro itself is quite non-trivial, but completely standard, a good explanation of how it works
// can be found here: https://github.com/swansontec/map-macro.
//
// Once we have a map macro all reflection is a matter of simply mapping __VA_ARGS__ into various
// arrays and tuples, which allows us to work with structures in a generic tuple-like manner.
//
// Partial specialization allows for a pretty concise implementation and provides
// nice error messages due to 'static_assert' on incorrect template arguments.
//
// An alternative frequently used way to do struct reflection is through generated code with structured binding
// & hundreds of overloads. This has a benefit of producing nicer error messages on 'for_each()' however before
// C++20 the resulting implementation is exceedingly verbose and doesn't provide a way to get field name info.
// For a macro-free C++20 reflection see 'utl::reflect_struct'.

// ____________________ IMPLEMENTATION ____________________

namespace utl::describe_struct::impl {

// =================
// --- Map macro ---
// =================

#define utl_dscs_eval_0(...) __VA_ARGS__
#define utl_dscs_eval_1(...) utl_dscs_eval_0(utl_dscs_eval_0(utl_dscs_eval_0(__VA_ARGS__)))
#define utl_dscs_eval_2(...) utl_dscs_eval_1(utl_dscs_eval_1(utl_dscs_eval_1(__VA_ARGS__)))
#define utl_dscs_eval_3(...) utl_dscs_eval_2(utl_dscs_eval_2(utl_dscs_eval_2(__VA_ARGS__)))
#define utl_dscs_eval_4(...) utl_dscs_eval_3(utl_dscs_eval_3(utl_dscs_eval_3(__VA_ARGS__)))
#define utl_dscs_eval(...) utl_dscs_eval_4(utl_dscs_eval_4(utl_dscs_eval_4(__VA_ARGS__)))

#define utl_dscs_map_end(...)
#define utl_dscs_map_out
#define utl_dscs_map_comma ,

#define utl_dscs_map_get_end_2() 0, utl_dscs_map_end
#define utl_dscs_map_get_end_1(...) utl_dscs_map_get_end_2
#define utl_dscs_map_get_end(...) utl_dscs_map_get_end_1
#define utl_dscs_map_next_0(test, next, ...) next utl_dscs_map_out
#define utl_dscs_map_next_1(test, next) utl_dscs_map_next_0(test, next, 0)
#define utl_dscs_map_next(test, next) utl_dscs_map_next_1(utl_dscs_map_get_end test, next)

#define utl_dscs_map_0(f, x, peek, ...) f(x) utl_dscs_map_next(peek, utl_dscs_map_1)(f, peek, __VA_ARGS__)
#define utl_dscs_map_1(f, x, peek, ...) f(x) utl_dscs_map_next(peek, utl_dscs_map_0)(f, peek, __VA_ARGS__)

#define utl_dscs_map_list_next_1(test, next) utl_dscs_map_next_0(test, utl_dscs_map_comma next, 0)
#define utl_dscs_map_list_next(test, next) utl_dscs_map_list_next_1(utl_dscs_map_get_end test, next)

#define utl_dscs_map_list_0(f, x, peek, ...)                                                                           \
    f(x) utl_dscs_map_list_next(peek, utl_dscs_map_list_1)(f, peek, __VA_ARGS__)
#define utl_dscs_map_list_1(f, x, peek, ...)                                                                           \
    f(x) utl_dscs_map_list_next(peek, utl_dscs_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro 'f' to all '__VA_ARGS__'
#define utl_dscs_map(f, ...) utl_dscs_eval(utl_dscs_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro 'f' to to all '__VA_ARGS__' and inserts commas between the results
#define utl_dscs_map_list(f, ...) utl_dscs_eval(utl_dscs_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Note: 'dscs' is short for 'describe_struct'

// ============================
// --- Reflection mechanism ---
// ============================

// Helper function to create 'pair<name, reference-to-member>' entries, similar to 'std::forward_as_tuple()'
template <class T>
constexpr std::pair<std::string_view, T&&> forward_as_entry(std::string_view label, T&& value) noexcept {
    return std::pair<std::string_view, T&&>(label, std::forward<T>(value));
}

template <class>
constexpr bool always_false_v = false;

template <class S>
struct meta {
    static_assert(always_false_v<S>,
                  "Provided struct does not have a defined reflection. Use 'UTL_DESCRIBE_STRUCT' macro to define one.");
    // makes instantiation of this template a compile-time error
};

// Helper macros for codegen
#define utl_dscs_make_name(arg_) std::string_view(#arg_)
#define utl_dscs_fwd_value(arg_) std::forward<T>(val).arg_
#define utl_dscs_fwd_entry(arg_) forward_as_entry(std::string_view(#arg_), std::forward<T>(val).arg_)

#define UTL_DESCRIBE_STRUCT(struct_name_, ...)                                                                         \
    template <>                                                                                                        \
    struct utl::describe_struct::impl::meta<struct_name_> {                                                            \
        constexpr static std::string_view type_name = #struct_name_;                                                   \
                                                                                                                       \
        constexpr static auto names = std::array{utl_dscs_map_list(utl_dscs_make_name, __VA_ARGS__)};                  \
                                                                                                                       \
        template <class T>                                                                                             \
        constexpr static auto value_view(T&& val) noexcept {                                                           \
            return std::forward_as_tuple(utl_dscs_map_list(utl_dscs_fwd_value, __VA_ARGS__));                          \
        }                                                                                                              \
                                                                                                                       \
        template <class T>                                                                                             \
        constexpr static auto entry_view(T&& val) noexcept {                                                           \
            return std::make_tuple(utl_dscs_map_list(utl_dscs_fwd_entry, __VA_ARGS__));                                \
        }                                                                                                              \
    }

// =================
// --- Tuple API ---
// =================

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>; //  backport from C++20

template <class T>
[[nodiscard]] constexpr auto label_view(T&& = T{}) noexcept {
    return meta<remove_cvref_t<T>>::names;
}

template <class T>
[[nodiscard]] constexpr auto value_view(T&& structure) noexcept {
    return meta<remove_cvref_t<T>>::value_view(std::forward<T>(structure));
}

template <class T>
[[nodiscard]] constexpr auto entry_view(T&& structure) noexcept {
    return meta<remove_cvref_t<T>>::entry_view(std::forward<T>(structure));
}

// ==========================
// --- General reflection ---
// ==========================

template <class T>
constexpr std::string_view name = meta<T>::type_name;

template <class T>
constexpr std::size_t size = std::tuple_size_v<decltype(meta<T>::names)>;

// =========================
// --- Member reflection ---
// =========================

template <std::size_t N, class T>
[[nodiscard]] constexpr auto label(T&& = T{}) noexcept {
    return std::get<N>(label_view<T>());
}

template <std::size_t N, class T>
[[nodiscard]] constexpr decltype(auto) value(T&& structure) noexcept {
    return std::get<N>(value_view(std::forward<T>(structure)));
}

template <std::size_t N, class T>
[[nodiscard]] constexpr auto entry(T&& structure) noexcept {
    return std::get<N>(entry_view(std::forward<T>(structure)));
}

// ==================
// --- Algorithms ---
// ==================

// clang-format off

template <std::size_t value>
using index_constant = std::integral_constant<std::size_t, value>;

template <std::size_t... indices, class F>
constexpr void for_sequence(std::index_sequence<indices...>, F&& f)
    noexcept(noexcept((f(index_constant<indices>{}), ...)))
{
    static_assert(
        std::conjunction_v<std::is_invocable<F, index_constant<indices>>...>,
        "for_sequence() requires function to be invocable for every integral_constant<> in range"
    );
    
    (f(index_constant<indices>{}), ...); // ideally we would use 'std::invoke()' but it's not constexpr before C++20
}

template <class T, class F>
constexpr void for_indices(F&& f)
    noexcept(noexcept(for_sequence(std::make_index_sequence<size<T>>{}, std::forward<F>(f))))
{
    for_sequence(std::make_index_sequence<size<T>>{}, std::forward<F>(f));
}

// clang-format on

} // namespace utl::describe_struct::impl

// ______________________ PUBLIC API ______________________

namespace utl::describe_struct {

// macro -> UTL_DESCRIBE_STRUCT

using impl::size;
using impl::name;

using impl::label;
using impl::value;
using impl::entry;

using impl::label_view;
using impl::value_view;
using impl::entry_view;

using impl::for_indices;

} // namespace utl::describe_struct

#endif
#endif // module utl::describe_struct
