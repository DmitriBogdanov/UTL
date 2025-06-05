// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::enum_reflect
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_enum_reflect.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_ENUM_REFLECT)
#ifndef UTLHEADERGUARD_ENUM_REFLECT
#define UTLHEADERGUARD_ENUM_REFLECT

#define UTL_ENUM_REFLECT_VERSION_MAJOR 1
#define UTL_ENUM_REFLECT_VERSION_MINOR 0
#define UTL_ENUM_REFLECT_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <cstddef>     // size_t
#include <stdexcept>   // out_of_range
#include <string>      // string
#include <string_view> // string_view
#include <tuple>       // tuple_size_v<>
#include <type_traits> // underlying_type_t<>, enable_if_t<>, is_enum_v<>
#include <utility>     // pair<>

// ____________________ DEVELOPER DOCS ____________________

// Reflection mechanism is based entirely around the map macro and a single struct with partial specialization for the
// reflected enum. Map macro itself is quire non-trivial, but completely standard, a good explanation of how it works
// can be found here: https://github.com/swansontec/map-macro
//
// Once we have a map macro all reflection is a matter of simply mapping __VA_ARGS__ into a few "metadata"
// arrays which we will then traverse to perform string conversions.
//
// Partial specialization allows for a pretty concise implementation and provides nice error messages due to
// 'static_assert()' on incorrect template arguments.
//
// An alternative frequently used way to do enum reflection is through constexpr parsing of strings returned by
// compiler-specific '__PRETTY_FUNCTION__' and '__FUNCSIG__', it has a benefit of not requiring the reflection
// macro however it hammers compile times and imposes restrictions on enum values. Some issues such as binary
// bloat and bitflag-enums can be worked around through proper implementation and some conditional metadata
// templates, however such approach tends to be quite complex.

// ____________________ IMPLEMENTATION ____________________

namespace utl::enum_reflect::impl {

// =================
// --- Map macro ---
// =================

#define utl_erfl_eval_0(...) __VA_ARGS__
#define utl_erfl_eval_1(...) utl_erfl_eval_0(utl_erfl_eval_0(utl_erfl_eval_0(__VA_ARGS__)))
#define utl_erfl_eval_2(...) utl_erfl_eval_1(utl_erfl_eval_1(utl_erfl_eval_1(__VA_ARGS__)))
#define utl_erfl_eval_3(...) utl_erfl_eval_2(utl_erfl_eval_2(utl_erfl_eval_2(__VA_ARGS__)))
#define utl_erfl_eval_4(...) utl_erfl_eval_3(utl_erfl_eval_3(utl_erfl_eval_3(__VA_ARGS__)))
#define utl_erfl_eval(...) utl_erfl_eval_4(utl_erfl_eval_4(utl_erfl_eval_4(__VA_ARGS__)))

#define utl_erfl_map_end(...)
#define utl_erfl_map_out
#define utl_erfl_map_comma ,

#define utl_erfl_map_get_end_2() 0, utl_erfl_map_end
#define utl_erfl_map_get_end_1(...) utl_erfl_map_get_end_2
#define utl_erfl_map_get_end(...) utl_erfl_map_get_end_1
#define utl_erfl_map_next_0(test, next, ...) next utl_erfl_map_out
#define utl_erfl_map_next_1(test, next) utl_erfl_map_next_0(test, next, 0)
#define utl_erfl_map_next(test, next) utl_erfl_map_next_1(utl_erfl_map_get_end test, next)

#define utl_erfl_map_0(f, x, peek, ...) f(x) utl_erfl_map_next(peek, utl_erfl_map_1)(f, peek, __VA_ARGS__)
#define utl_erfl_map_1(f, x, peek, ...) f(x) utl_erfl_map_next(peek, utl_erfl_map_0)(f, peek, __VA_ARGS__)

#define utl_erfl_map_list_next_1(test, next) utl_erfl_map_next_0(test, utl_erfl_map_comma next, 0)
#define utl_erfl_map_list_next(test, next) utl_erfl_map_list_next_1(utl_erfl_map_get_end test, next)

#define utl_erfl_map_list_0(f, x, peek, ...)                                                                           \
    f(x) utl_erfl_map_list_next(peek, utl_erfl_map_list_1)(f, peek, __VA_ARGS__)
#define utl_erfl_map_list_1(f, x, peek, ...)                                                                           \
    f(x) utl_erfl_map_list_next(peek, utl_erfl_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro 'f' to all '__VA_ARGS__'
#define utl_erfl_map(f, ...) utl_erfl_eval(utl_erfl_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro 'f' to to all '__VA_ARGS__' and inserts commas between the results
#define utl_erfl_map_list(f, ...) utl_erfl_eval(utl_erfl_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Note: 'erfl' is short for 'enum_reflect'

// ============================
// --- Reflection mechanism ---
// ============================

template <class>
constexpr bool always_false_v = false;

template <class Enum>
struct Meta {
    static_assert(always_false_v<Enum>,
                  "Provided enum does not have a defined reflection. Use 'UTL_ENUM_REFLECT' macro to define one.");
    // makes instantiation of this template a compile-time error
};

// Helper macros for codegen
#define utl_erfl_make_value(arg_) type::arg_
#define utl_erfl_make_name(arg_) std::string_view(#arg_)
#define utl_erfl_make_entry(arg_) std::make_pair(std::string_view(#arg_), type::arg_)

#define UTL_ENUM_REFLECT(enum_name_, ...)                                                                              \
    template <>                                                                                                        \
    struct utl::enum_reflect::impl::Meta<enum_name_> {                                                                 \
        using type = enum_name_;                                                                                       \
                                                                                                                       \
        constexpr static std::string_view type_name = #enum_name_;                                                     \
                                                                                                                       \
        constexpr static auto names   = std::array{utl_erfl_map_list(utl_erfl_make_name, __VA_ARGS__)};                \
        constexpr static auto values  = std::array{utl_erfl_map_list(utl_erfl_make_value, __VA_ARGS__)};               \
        constexpr static auto entries = std::array{utl_erfl_map_list(utl_erfl_make_entry, __VA_ARGS__)};               \
    }

// ======================
// --- Reflection API ---
// ======================

template <class Enum>
constexpr auto type_name = Meta<Enum>::type_name;

template <class Enum>
constexpr auto names = Meta<Enum>::names;

template <class Enum>
constexpr auto values = Meta<Enum>::values;

template <class Enum>
constexpr auto entries = Meta<Enum>::entries;

template <class Enum>
constexpr auto size = std::tuple_size_v<decltype(values<Enum>)>;

template <class Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
[[nodiscard]] constexpr auto to_underlying(Enum value) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(value);
    // doesn't really require reflection, but might as well have it here,
    // in C++23 gets replaced by 'std::to_underlying'
}

template <class Enum>
[[nodiscard]] constexpr bool is_valid(Enum value) noexcept {
    for (const auto& e : values<Enum>)
        if (value == e) return true;
    return false;
}

template <class Enum>
[[nodiscard]] constexpr std::string_view to_string(Enum val) {
    for (const auto& [name, value] : entries<Enum>)
        if (val == value) return name;

    throw std::out_of_range("enum_reflect::to_string<" + std::string(type_name<Enum>) + ">(): value " +
                            std::to_string(to_underlying(val)) + " is not a part of enumeration.");
}

template <class Enum>
[[nodiscard]] constexpr Enum from_string(std::string_view str) {
    for (const auto& [name, value] : entries<Enum>)
        if (str == name) return value;

    throw std::out_of_range("enum_reflect::from_string<" + std::string(type_name<Enum>) + ">(): name \"" +
                            std::string(str) + "\" is not a part of enumeration.");
}

} // namespace utl::enum_reflect::impl

// ______________________ PUBLIC API ______________________

namespace utl::enum_reflect {

// macro -> UTL_ENUM_REFLECT

using impl::type_name;
using impl::size;

using impl::names;
using impl::values;
using impl::entries;

using impl::is_valid;
using impl::to_underlying;

using impl::to_string;
using impl::from_string;

} // namespace utl::enum_reflect

#endif
#endif // module utl::enum_reflect
