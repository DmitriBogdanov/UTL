// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::struct_reflect
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_struct_reflect.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRUCT_REFLECT)
#ifndef UTLHEADERGUARD_STRUCT_REFLECT
#define UTLHEADERGUARD_STRUCT_REFLECT

#define UTL_STRUCT_REFLECT_VERSION_MAJOR 1
#define UTL_STRUCT_REFLECT_VERSION_MINOR 0
#define UTL_STRUCT_REFLECT_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <cstddef>     // size_t
#include <string_view> // string_view
#include <tuple>       // tuple<>, tuple_size<>, apply<>(), get<>()
#include <type_traits> // add_lvalue_reference_t<>, add_const_t<>, remove_reference_t<>, decay_t<>
#include <utility>     // forward<>(), pair<>

// ____________________ DEVELOPER DOCS ____________________

// Reflection mechanism is based entirely around the map macro and a single struct with partial specialization for the
// reflected enum. Map macro itself is quire non-trivial, but completely standard, a good explanation of how it works
// can be found here: [https://github.com/swansontec/map-macro].
//
// Once we have a map macro all reflection is a matter of simply mapping __VA_ARGS__ into various
// arrays and tuples, which allows us to work with structures in a generic tuple-like way.
//
// Partial specialization allows for a pretty concise implementation and provides nice error messages due to
// static_assert on incorrect template arguments.
//
// An alternative frequently used way to do struct reflection is through generated code with structured binding &
// hundreds of overloads. This has a benefit of producing nicer error messages on 'for_each()' however the
// resulting implementation is exceedingly verbose and doesn't provide field name info.

// ____________________ IMPLEMENTATION ____________________

namespace utl::struct_reflect::impl {

// =================
// --- Map macro ---
// =================

#define utl_srfl_eval_0(...) __VA_ARGS__
#define utl_srfl_eval_1(...) utl_srfl_eval_0(utl_srfl_eval_0(utl_srfl_eval_0(__VA_ARGS__)))
#define utl_srfl_eval_2(...) utl_srfl_eval_1(utl_srfl_eval_1(utl_srfl_eval_1(__VA_ARGS__)))
#define utl_srfl_eval_3(...) utl_srfl_eval_2(utl_srfl_eval_2(utl_srfl_eval_2(__VA_ARGS__)))
#define utl_srfl_eval_4(...) utl_srfl_eval_3(utl_srfl_eval_3(utl_srfl_eval_3(__VA_ARGS__)))
#define utl_srfl_eval(...) utl_srfl_eval_4(utl_srfl_eval_4(utl_srfl_eval_4(__VA_ARGS__)))

#define utl_srfl_map_end(...)
#define utl_srfl_map_out
#define utl_srfl_map_comma ,

#define utl_srfl_map_get_end_2() 0, utl_srfl_map_end
#define utl_srfl_map_get_end_1(...) utl_srfl_map_get_end_2
#define utl_srfl_map_get_end(...) utl_srfl_map_get_end_1
#define utl_srfl_map_next_0(test, next, ...) next utl_srfl_map_out
#define utl_srfl_map_next_1(test, next) utl_srfl_map_next_0(test, next, 0)
#define utl_srfl_map_next(test, next) utl_srfl_map_next_1(utl_srfl_map_get_end test, next)

#define utl_srfl_map_0(f, x, peek, ...) f(x) utl_srfl_map_next(peek, utl_srfl_map_1)(f, peek, __VA_ARGS__)
#define utl_srfl_map_1(f, x, peek, ...) f(x) utl_srfl_map_next(peek, utl_srfl_map_0)(f, peek, __VA_ARGS__)

#define utl_srfl_map_list_next_1(test, next) utl_srfl_map_next_0(test, utl_srfl_map_comma next, 0)
#define utl_srfl_map_list_next(test, next) utl_srfl_map_list_next_1(utl_srfl_map_get_end test, next)

#define utl_srfl_map_list_0(f, x, peek, ...)                                                                           \
    f(x) utl_srfl_map_list_next(peek, utl_srfl_map_list_1)(f, peek, __VA_ARGS__)
#define utl_srfl_map_list_1(f, x, peek, ...)                                                                           \
    f(x) utl_srfl_map_list_next(peek, utl_srfl_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro 'f' to all '__VA_ARGS__'
#define utl_srfl_map(f, ...) utl_srfl_eval(utl_srfl_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro 'f' to to all '__VA_ARGS__' and inserts commas between the results
#define utl_srfl_map_list(f, ...) utl_srfl_eval(utl_srfl_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Note: 'srfl' is short for 'struct_reflect'

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
struct Meta {
    static_assert(always_false_v<S>,
                  "Provided struct does not have a defined reflection. Use 'UTL_STRUCT_REFLECT' macro to define one.");
    // makes instantiation of this template a compile-time error
};

// Helper macros for codegen
#define utl_srfl_make_name(arg_) std::string_view(#arg_)
#define utl_srfl_fwd_value(arg_) std::forward<S>(val).arg_
#define utl_srfl_fwd_entry(arg_) make_entry(std::string_view(#arg_), std::forward<S>(val).arg_)

#define utl_srfl_call_unary_func(arg_) func(std::forward<S>(val).arg_);
#define utl_srfl_call_binary_func(arg_) func(std::forward<S1>(val_1).arg_, std::forward<S2>(val_2).arg_);
#define utl_srfl_and_unary_predicate(arg_) &&func(val.arg_)
#define utl_srfl_and_binary_predicate(arg_) &&func(val_1.arg_, val_2.arg_)

#define UTL_STRUCT_REFLECT(struct_name_, ...)                                                                          \
    template <>                                                                                                        \
    struct utl::struct_reflect::impl::Meta<struct_name_> {                                                             \
        constexpr static std::string_view type_name = #struct_name_;                                                   \
                                                                                                                       \
        constexpr static auto names = std::array{utl_srfl_map_list(utl_srfl_make_name, __VA_ARGS__)};                  \
                                                                                                                       \
        template <class S>                                                                                             \
        constexpr static auto field_view(S&& val) noexcept {                                                           \
            return std::forward_as_tuple(utl_srfl_map_list(utl_srfl_fwd_value, __VA_ARGS__));                          \
        }                                                                                                              \
                                                                                                                       \
        template <class S>                                                                                             \
        constexpr static auto entry_view(S&& val) noexcept {                                                           \
            return std::make_tuple(utl_srfl_map_list(utl_srfl_fwd_entry, __VA_ARGS__));                                \
        }                                                                                                              \
                                                                                                                       \
        template <class S, class Func>                                                                                 \
        constexpr static void for_each(S&& val, Func&& func) {                                                         \
            utl_srfl_map(utl_srfl_call_unary_func, __VA_ARGS__)                                                        \
        }                                                                                                              \
                                                                                                                       \
        template <class S1, class S2, class Func>                                                                      \
        constexpr static void for_each(S1&& val_1, S2&& val_2, Func&& func) {                                          \
            utl_srfl_map(utl_srfl_call_binary_func, __VA_ARGS__)                                                       \
        }                                                                                                              \
                                                                                                                       \
        template <class S, class Func>                                                                                 \
        constexpr static bool true_for_all(const S& val, Func&& func) {                                                \
            return true utl_srfl_map(utl_srfl_and_unary_predicate, __VA_ARGS__);                                       \
        }                                                                                                              \
                                                                                                                       \
        template <class S1, class S2, class Func>                                                                      \
        constexpr static bool true_for_all(const S1& val_1, const S2& val_2, Func&& func) {                            \
            return true utl_srfl_map(utl_srfl_and_binary_predicate, __VA_ARGS__);                                      \
        }                                                                                                              \
    }

// Note: 'true' in front of a generated predicate chain handles the redundant '&&' at the beginning

// ======================
// --- Reflection API ---
// ======================

template <class S>
constexpr auto type_name = Meta<S>::type_name;

template <class S>
constexpr auto names = Meta<S>::names;

template <class S>
constexpr auto field_view(S&& value) noexcept {
    using struct_type = typename std::decay_t<S>;
    return Meta<struct_type>::field_view(std::forward<S>(value));
}

template <class S>
constexpr auto entry_view(S&& value) noexcept {
    using struct_type = typename std::decay_t<S>;
    return Meta<struct_type>::entry_view(std::forward<S>(value));
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
    Meta<struct_type>::for_each(std::forward<S>(value), std::forward<Func>(func));
}

template <class S1, class S2, class Func>
constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func) {
    using struct_type_1 = typename std::decay_t<S1>;
    using struct_type_2 = typename std::decay_t<S2>;
    static_assert(std::is_same_v<struct_type_1, struct_type_2>,
                  "Called 'struct_reflect::for_each(s1, s2, func)' with incompatible argument types.");
    Meta<struct_type_1>::for_each(std::forward<S1>(value_1), std::forward<S2>(value_2), std::forward<Func>(func));
}

// Predicate checks cannot be efficiently implemented in terms of 'for_each()'
// we use a separate implementation with short-circuiting
template <class S, class Func>
constexpr bool true_for_all(const S& value, Func&& func) {
    using struct_type = typename std::decay_t<S>;
    return Meta<struct_type>::true_for_all(value, std::forward<Func>(func));
}

template <class S1, class S2, class Func>
constexpr bool true_for_all(const S1& value_1, const S2& value_2, Func&& func) {
    using struct_type_1 = typename std::decay_t<S1>;
    using struct_type_2 = typename std::decay_t<S2>;
    static_assert(std::is_same_v<struct_type_1, struct_type_2>,
                  "Called 'struct_reflect::for_each(s1, s2, func)' with incompatible argument types.");
    return Meta<struct_type_1>::true_for_all(value_1, value_2, std::forward<Func>(func));
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
                  "Called 'struct_reflect::tuple_for_each(t1, t2, func)' with incompatible tuple sizes.");
    tuple_for_each_impl(std::forward<T1>(tuple_1), std::forward<T2>(tuple_2), std::forward<Func>(func),
                        std::make_index_sequence<tuple_size_1>{});
}

} // namespace utl::struct_reflect::impl

// ______________________ PUBLIC API ______________________

namespace utl::struct_reflect {

// macro -> UTL_STRUCT_REFLECT

using impl::type_name;
using impl::size;

using impl::names;
using impl::field_view;
using impl::entry_view;

using impl::get;

using impl::for_each;
using impl::true_for_all;

using impl::tuple_for_each;

} // namespace utl::struct_reflect

#endif
#endif // module utl::struct_reflect
