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
#define UTL_DESCRIBE_STRUCT_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>       // IWYU pragma: keep (used in a macro) | array<>
#include <cstddef>     // size_t
#include <string_view> // IWYU pragma: keep (used in a macro) | string_view
#include <tuple>       // tuple<>, tuple_size<>, apply<>(), get<>()
#include <type_traits> // add_lvalue_reference_t<>, add_const_t<>, remove_reference_t<>, decay_t<>
#include <utility>     // forward<>(), pair<>

// ____________________ DEVELOPER DOCS ____________________

// Reflection mechanism is based entirely around the map macro and a single struct with partial specialization for the
// reflected enum. Map macro itself is quire non-trivial, but completely standard, a good explanation of how it works
// can be found here: https://github.com/swansontec/map-macro.
//
// Once we have a map macro all reflection is a matter of simply mapping __VA_ARGS__ into various
// arrays and tuples, which allows us to work with structures in a generic tuple-like way.
//
// Partial specialization allows for a pretty concise implementation and provides nice error messages due to
// static_assert on incorrect template arguments.
//
// An alternative frequently used way to do struct reflection is through generated code with structured binding
// & hundreds of overloads. This has a benefit of producing nicer error messages on 'for_each()' however before
// C++20 the resulting implementation is exceedingly verbose and doesn't provide a way to get field name info.
// For macro-free C++20 reflection see 'utl::reflect_struct'.

// ____________________ IMPLEMENTATION ____________________

namespace utl::describe_struct::impl {

// =================
// --- Map macro ---
// =================

#define utl_dcst_eval_0(...) __VA_ARGS__
#define utl_dcst_eval_1(...) utl_dcst_eval_0(utl_dcst_eval_0(utl_dcst_eval_0(__VA_ARGS__)))
#define utl_dcst_eval_2(...) utl_dcst_eval_1(utl_dcst_eval_1(utl_dcst_eval_1(__VA_ARGS__)))
#define utl_dcst_eval_3(...) utl_dcst_eval_2(utl_dcst_eval_2(utl_dcst_eval_2(__VA_ARGS__)))
#define utl_dcst_eval_4(...) utl_dcst_eval_3(utl_dcst_eval_3(utl_dcst_eval_3(__VA_ARGS__)))
#define utl_dcst_eval(...) utl_dcst_eval_4(utl_dcst_eval_4(utl_dcst_eval_4(__VA_ARGS__)))

#define utl_dcst_map_end(...)
#define utl_dcst_map_out
#define utl_dcst_map_comma ,

#define utl_dcst_map_get_end_2() 0, utl_dcst_map_end
#define utl_dcst_map_get_end_1(...) utl_dcst_map_get_end_2
#define utl_dcst_map_get_end(...) utl_dcst_map_get_end_1
#define utl_dcst_map_next_0(test, next, ...) next utl_dcst_map_out
#define utl_dcst_map_next_1(test, next) utl_dcst_map_next_0(test, next, 0)
#define utl_dcst_map_next(test, next) utl_dcst_map_next_1(utl_dcst_map_get_end test, next)

#define utl_dcst_map_0(f, x, peek, ...) f(x) utl_dcst_map_next(peek, utl_dcst_map_1)(f, peek, __VA_ARGS__)
#define utl_dcst_map_1(f, x, peek, ...) f(x) utl_dcst_map_next(peek, utl_dcst_map_0)(f, peek, __VA_ARGS__)

#define utl_dcst_map_list_next_1(test, next) utl_dcst_map_next_0(test, utl_dcst_map_comma next, 0)
#define utl_dcst_map_list_next(test, next) utl_dcst_map_list_next_1(utl_dcst_map_get_end test, next)

#define utl_dcst_map_list_0(f, x, peek, ...)                                                                           \
    f(x) utl_dcst_map_list_next(peek, utl_dcst_map_list_1)(f, peek, __VA_ARGS__)
#define utl_dcst_map_list_1(f, x, peek, ...)                                                                           \
    f(x) utl_dcst_map_list_next(peek, utl_dcst_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro 'f' to all '__VA_ARGS__'
#define utl_dcst_map(f, ...) utl_dcst_eval(utl_dcst_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro 'f' to to all '__VA_ARGS__' and inserts commas between the results
#define utl_dcst_map_list(f, ...) utl_dcst_eval(utl_dcst_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Note: 'dcst' is short for 'describe_struct'

// ============================
// --- Reflection mechanism ---
// ============================

template <class T1, class T2>
constexpr std::pair<T1, T2&&> make_entry(T1&& a, T2&& b) noexcept {
    return std::pair<T1, T2&&>(std::forward<T1>(a), std::forward<T2>(b));
    // helper function used to create < name, reference-to-field > entries
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
#define utl_dcst_make_name(arg_) std::string_view(#arg_)
#define utl_dcst_fwd_value(arg_) std::forward<S>(val).arg_
#define utl_dcst_fwd_entry(arg_) make_entry(std::string_view(#arg_), std::forward<S>(val).arg_)

#define utl_dcst_call_unary_func(arg_) func(std::forward<S>(val).arg_);
#define utl_dcst_call_binary_func(arg_) func(std::forward<S1>(val_1).arg_, std::forward<S2>(val_2).arg_);
#define utl_dcst_and_unary_predicate(arg_) &&func(val.arg_)
#define utl_dcst_and_binary_predicate(arg_) &&func(val_1.arg_, val_2.arg_)

#define UTL_DESCRIBE_STRUCT(struct_name_, ...)                                                                         \
    template <>                                                                                                        \
    struct utl::describe_struct::impl::meta<struct_name_> {                                                            \
        constexpr static std::string_view type_name = #struct_name_;                                                   \
                                                                                                                       \
        constexpr static auto names = std::array{utl_dcst_map_list(utl_dcst_make_name, __VA_ARGS__)};                  \
                                                                                                                       \
        template <class S>                                                                                             \
        constexpr static auto field_view(S&& val) noexcept {                                                           \
            return std::forward_as_tuple(utl_dcst_map_list(utl_dcst_fwd_value, __VA_ARGS__));                          \
        }                                                                                                              \
                                                                                                                       \
        template <class S>                                                                                             \
        constexpr static auto entry_view(S&& val) noexcept {                                                           \
            return std::make_tuple(utl_dcst_map_list(utl_dcst_fwd_entry, __VA_ARGS__));                                \
        }                                                                                                              \
                                                                                                                       \
        template <class S, class Func>                                                                                 \
        constexpr static void for_each(S&& val, Func&& func) {                                                         \
            utl_dcst_map(utl_dcst_call_unary_func, __VA_ARGS__)                                                        \
        }                                                                                                              \
                                                                                                                       \
        template <class S1, class S2, class Func>                                                                      \
        constexpr static void for_each(S1&& val_1, S2&& val_2, Func&& func) {                                          \
            utl_dcst_map(utl_dcst_call_binary_func, __VA_ARGS__)                                                       \
        }                                                                                                              \
                                                                                                                       \
        template <class S, class Func>                                                                                 \
        constexpr static bool true_for_all(const S& val, Func&& func) {                                                \
            return true utl_dcst_map(utl_dcst_and_unary_predicate, __VA_ARGS__);                                       \
        }                                                                                                              \
                                                                                                                       \
        template <class S1, class S2, class Func>                                                                      \
        constexpr static bool true_for_all(const S1& val_1, const S2& val_2, Func&& func) {                            \
            return true utl_dcst_map(utl_dcst_and_binary_predicate, __VA_ARGS__);                                      \
        }                                                                                                              \
    }

// Note: 'true' in front of a generated predicate chain handles the redundant '&&' at the beginning

// ======================
// --- Reflection API ---
// ======================

template <class S>
constexpr auto type_name = meta<S>::type_name;

template <class S>
constexpr auto names = meta<S>::names;

template <class S>
constexpr auto field_view(S&& value) noexcept {
    using struct_type = typename std::decay_t<S>;
    return meta<struct_type>::field_view(std::forward<S>(value));
}

template <class S>
constexpr auto entry_view(S&& value) noexcept {
    using struct_type = typename std::decay_t<S>;
    return meta<struct_type>::entry_view(std::forward<S>(value));
}

template <class S>
constexpr auto size = std::tuple_size_v<decltype(names<S>)>;

template <std::size_t I, class S>
constexpr auto get(S&& value) noexcept {
    return std::get<I>(field_view(std::forward<S>(value)));
}

template <class S, class Func>
constexpr void for_each(S&& value, Func&& func) {
    using struct_type = typename std::decay_t<S>;
    meta<struct_type>::for_each(std::forward<S>(value), std::forward<Func>(func));
}

template <class S1, class S2, class Func>
constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func) {
    using struct_type_1 = typename std::decay_t<S1>;
    using struct_type_2 = typename std::decay_t<S2>;
    static_assert(std::is_same_v<struct_type_1, struct_type_2>,
                  "Called 'describe_struct::for_each(s1, s2, func)' with incompatible argument types.");
    meta<struct_type_1>::for_each(std::forward<S1>(value_1), std::forward<S2>(value_2), std::forward<Func>(func));
}

// Predicate checks cannot be efficiently implemented in terms of 'for_each()'
// we use a separate implementation with short-circuiting
template <class S, class Func>
constexpr bool true_for_all(const S& value, Func&& func) {
    using struct_type = typename std::decay_t<S>;
    return meta<struct_type>::true_for_all(value, std::forward<Func>(func));
}

template <class S1, class S2, class Func>
constexpr bool true_for_all(const S1& value_1, const S2& value_2, Func&& func) {
    using struct_type_1 = typename std::decay_t<S1>;
    using struct_type_2 = typename std::decay_t<S2>;
    static_assert(std::is_same_v<struct_type_1, struct_type_2>,
                  "Called 'describe_struct::for_each(s1, s2, func)' with incompatible argument types.");
    return meta<struct_type_1>::true_for_all(value_1, value_2, std::forward<Func>(func));
}

// --- Misc utils ---
// ------------------

// Struct reflection provides its own 'for_each()' with no tuple magic, this function is useful
// in case user want to operate on tuples rather than structs using similar API, sort of a "bonus utility"
// that simply doesn't have any better module to be a part of
template <class T, class Func>
constexpr void tuple_for_each(T&& tuple, Func&& func) {
    std::apply([&func](auto&&... args) { (func(std::forward<decltype(args)>(args)), ...); }, std::forward<T>(tuple));
}

// For a pair of tuple 'std::apply' trick doesn't cut it, gotta do the standard thing
// with recursion over the index sequence. This looks a little horrible, but no too much
template <class T1, class T2, class Func, std::size_t... Idx>
constexpr void tuple_for_each_impl(T1&& tuple_1, T2&& tuple_2, Func&& func, std::index_sequence<Idx...>) {
    (func(std::get<Idx>(std::forward<T1>(tuple_1)), std::get<Idx>(std::forward<T2>(tuple_2))), ...);
    // fold expression '( f(args), ... )' invokes 'f(args)' for all indices in the index sequence
}

template <class T1, class T2, class Func>
constexpr void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func) {
    constexpr std::size_t tuple_size_1 = std::tuple_size_v<std::decay_t<T1>>;
    constexpr std::size_t tuple_size_2 = std::tuple_size_v<std::decay_t<T2>>;

    static_assert(tuple_size_1 == tuple_size_2,
                  "Called 'describe_struct::tuple_for_each(t1, t2, func)' with incompatible tuple sizes.");

    tuple_for_each_impl(std::forward<T1>(tuple_1), std::forward<T2>(tuple_2), std::forward<Func>(func),
                        std::make_index_sequence<tuple_size_1>{});
}

} // namespace utl::describe_struct::impl

// ______________________ PUBLIC API ______________________

namespace utl::describe_struct {

// macro -> UTL_DESCRIBE_STRUCT

using impl::type_name;
using impl::size;

using impl::names;
using impl::field_view;
using impl::entry_view;

using impl::get;

using impl::for_each;
using impl::true_for_all;

using impl::tuple_for_each;

} // namespace utl::describe_struct

#endif
#endif // module utl::describe_struct
