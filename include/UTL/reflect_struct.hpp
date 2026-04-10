// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::reflect_struct
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_reflect_struct.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_REFLECT_STRUCT)

#ifndef utl_reflect_struct_headerguard
#define utl_reflect_struct_headerguard

#define UTL_REFLECT_STRUCT_VERSION_MAJOR 1
#define UTL_REFLECT_STRUCT_VERSION_MINOR 0
#define UTL_REFLECT_STRUCT_VERSION_PATCH 5

// _______________________ INCLUDES _______________________

#include <array>           // array<>, size_t
#include <functional>      // invoke()
#include <source_location> // source_location
#include <string_view>     // string_view
#include <tuple>           // tie(), tuple<>
#include <type_traits>     // is_class<>, is_base_of<>, is_aggregate<>, remove_cvref<>
#include <utility>         // pair<>, forward<>(), index_sequence<>, make_index_sequence<>

// ____________________ DEVELOPER DOCS ____________________

// This is a clean & minimal implementation of C++20 aggregate reflection.
// Similar technique can be found in Glaze, reflect-cpp and Boost.PFR,
// which makes a strong case for its robustness despite technically
// relying on the format of implementation-specific strings.

// ____________________ IMPLEMENTATION ____________________

namespace utl::reflect_struct::impl {

// =========================
// --- Aggregate concept ---
// =========================

// Since C++17 aggregate classes can have other aggregates as a base, this case is rare in practice
// and it makes reflection difficult due to base classes affecting semantics for aggregate initialization.
// To ensure strictness, we constrain all templates to only accept non-derived aggregates.

template <class Struct>
    requires std::is_class_v<Struct>
struct base_convertible {
    template <class T>
        requires std::is_base_of_v<T, Struct>
    operator T() const noexcept;
};

template <class T>
concept non_derived_aggregate = std::is_aggregate_v<T> && !requires { T{base_convertible<T>{}}; };

// =====================
// --- Count members ---
// =====================

// Aggregate members can be counted by recursively trying to compile aggregate initialization:
//    'Struct{ arbitrary_convertible{} }                         ' compiles => struct has at least 1 member
//    'Struct{ arbitrary_convertible{}, arbitrary_convertible{} }' compiles => struct has at least 2 members
//    ... and so on ...
// once compilation attempt fails, we can return the length of accumulated parameter pack.

struct arbitrary_convertible {
    template <class T>
    operator T() const noexcept;
};

template <non_derived_aggregate Struct, std::same_as<arbitrary_convertible>... Args>
constexpr std::size_t count_members = [] {
    if constexpr (requires { Struct{Args{}..., arbitrary_convertible{}}; }) {
        return count_members<Struct, Args..., arbitrary_convertible>;
    } else {
        return sizeof...(Args);
    }
}();

// Wrap for public API

template <class T>
    requires non_derived_aggregate<std::remove_cvref_t<T>>
constexpr std::size_t size = count_members<std::remove_cvref_t<T>>;

// ================
// --- To tuple ---
// ================

// To access struct members using an index, we need to interpret it as a tuple.
// This was possible even before C++20, but more cumbersome than the new algorithm:
//    1) Deduce struct size
//    2) Based on size decompose struct into variables with structured binding
//    3) Return a tie of those variables as a standard tuple
// This cannot be done generically, so we have to manually specify cases up to some reasonable limit.

constexpr std::size_t member_limit = 20;

template <class T>
concept reflectable = non_derived_aggregate<std::remove_cvref_t<T>> && size<T> <= member_limit;

template <reflectable T>
[[nodiscard]] constexpr decltype(auto) to_tie(T&& structure) {
    constexpr std::size_t N = size<T>;

    if constexpr (N == 0) {
        // <0 members>
        return std::tuple{};
    }
    if constexpr (N == 1) {
        auto& [m0] = structure;
        return std::tie(m0);
    }
    if constexpr (N == 2) {
        auto& [m0, m1] = structure;
        return std::tie(m0, m1);
    }
    if constexpr (N == 3) {
        auto& [m0, m1, m2] = structure;
        return std::tie(m0, m1, m2);
    }
    if constexpr (N == 4) {
        auto& [m0, m1, m2, m3] = structure;
        return std::tie(m0, m1, m2, m3);
    }
    if constexpr (N == 5) {
        auto& [m0, m1, m2, m3, m4] = structure;
        return std::tie(m0, m1, m2, m3, m4);
    }
    if constexpr (N == 6) {
        auto& [m0, m1, m2, m3, m4, m5] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5);
    }
    if constexpr (N == 7) {
        auto& [m0, m1, m2, m3, m4, m5, m6] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6);
    }
    if constexpr (N == 8) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7);
    }
    if constexpr (N == 9) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8);
    }
    if constexpr (N == 10) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9);
    }
    if constexpr (N == 11) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10);
    }
    if constexpr (N == 12) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
    }
    if constexpr (N == 13) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
    }
    if constexpr (N == 14) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
    }
    if constexpr (N == 15) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14);
    }
    if constexpr (N == 16) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15);
    }
    if constexpr (N == 17) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16);
    }
    if constexpr (N == 18) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17);
    }
    if constexpr (N == 19) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18);
    }
    if constexpr (N == 20) {
        auto& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19] = structure;
        return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19);
    }
}

// Wrap for public API

template <std::size_t N, reflectable T>
[[nodiscard]] constexpr decltype(auto) value(T&& structure) {
    return std::get<N>(to_tie(std::forward<T>(structure)));
}

// =========================
// --- Mangled type name ---
// =========================

// Template function source name can be used to get an implementation-defined
// string that contains a textual representation of the template parameter.
//
// Note that template parameters MUST be named even if we don't use them, otherwise
// some compilers (i.e. clang) might decide to omit them from string representation.

template <auto arg>
[[nodiscard]] consteval std::string_view mangled_value_name() noexcept {
#ifdef _MSC_VER
    return __FUNCSIG__;
    // workaround for MSVC IntelliSense, while the compiler itself works fine with 'std::source_location',
    // its IntelliSense uses an entirely separate engine which doesn't properly evaluate <source_locations>
    // (and a lot of other constexpr things) resulting in broken syntax highlighting
#endif
    return std::source_location::current().function_name();
}

template <class Arg>
[[nodiscard]] consteval std::string_view mangled_type_name() noexcept {
#ifdef _MSC_VER
    return __FUNCSIG__;
#endif
    return std::source_location::current().function_name();
}

[[nodiscard]] consteval std::size_t literal_size(const char* str) noexcept { return std::string_view{str}.size(); }

// ======================
// --- Member pointer ---
// ======================

template <class T>
struct pointer_wrapper {
    const T* ptr;
};

template <std::size_t N, reflectable Struct>
[[nodiscard]] constexpr auto get_ptr(Struct&& value) noexcept {
    auto& nth_member_reference = std::get<N>(to_tie(value));
    using nth_member_type      = std::remove_cvref_t<decltype(nth_member_reference)>;

    return pointer_wrapper<nth_member_type>{&nth_member_reference};
};

// =============================
// --- Field name extraction ---
// =============================

// 'extern' variable is the secret sauce that allows us to extract field names from a struct.
// If we wrap class into an 'extern' variable and generate mangled name of its member pointer
// we will get a string which contains corresponding field name. Below are example strings for
// all major compilers with 'utl::reflect_struct::impl::' namespace removed to reduce verbosity:
//
// GCC =>
//    "consteval std::string_view mangled_value_name() [with auto arg = pointer_wrapper<int>{
//     (& external<CLASS>.FIELD)}; std::string_view = std::basic_string_view<char>]"
//                       ^         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                  prefix         suffix
// clang =>
//   "std::string_view mangled_value_name() [arg = pointer_wrapper<nth_member_type>{&external.FIELD}]"
//                                                                                           ^     ^^
//                                                                                      prefix     suffix
// MSVC =>
//   "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl mangled_value_name
//    <struct pointer_wrapper<int>{const int*:&external<struct CLASS>->FIELD}>(void) noexcept"
//                                                                    ^     ^^^^^^^^^^^^^^^^^
//                                                               prefix     suffix   
//
// To parse this implementation-defined string in a (reasonably) cross-platform way without hard-coding 
// the prefixes, we can generate an example string for some known struct upon compilation and use it 
// to deduce implementation-defined prefixes / suffixes relative to the known 'FIELD'.

template <class T>
extern const T external{};

template <std::size_t N, reflectable T>
consteval std::string_view mangled_field_name() noexcept {
    return mangled_value_name<get_ptr<N>(external<T>)>();
}

struct example_struct {
    int example_field;
};

struct compiler_specifics_field {
    constexpr static std::string_view example = mangled_field_name<0, example_struct>();
    constexpr static std::size_t      offset  = example.find("example_field");
    constexpr static char             prefix  = example.at(offset - 1);
    constexpr static std::string_view suffix  = example.substr(offset + literal_size("example_field"));
};

template <std::size_t N, reflectable Structure>
constexpr std::string_view member_name = [] {
    constexpr std::string_view mangled  = mangled_field_name<N, Structure>();
    constexpr std::string_view prefixed = mangled.substr(0, mangled.find(compiler_specifics_field::suffix));
    constexpr std::string_view trimmed  = prefixed.substr(prefixed.find_last_of(compiler_specifics_field::prefix) + 1);
    return trimmed;
}();

// Wrap for public API

template <std::size_t N, reflectable T>
[[nodiscard]] constexpr std::string_view label(T&& = T{}) {
    return member_name<N, std::remove_cvref_t<T>>;
}

template <std::size_t N, reflectable T>
[[nodiscard]] constexpr auto entry(T&& structure) noexcept {
    using forwarding_pair_type = std::pair<std::string_view, decltype(value<N>(std::forward<T>(structure)))>;
    return forwarding_pair_type{label<N>(structure), value<N>(std::forward<T>(structure))};
}

// ============================
// --- Type name extraction ---
// ============================

// Same idea as in field name extraction, but simpler. Below are example strings for all
// major compilers with 'utl::reflect_struct::impl::' namespace removed to reduce verbosity:
//
// GCC =>
//   "consteval std::string_view mangled_type_name() [with Arg = TYPE; std::string_view = std::basic_string_view<char>]"
//                                                               ^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                                                          offset   suffix
// clang =>
//    "std::string_view mangled_type_name() [Arg = TYPE]"
//                                                 ^   ^
//                                            offset   suffix
// MSVC =>
//    "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl mangled_type_name<TYPE>(void) noexcept"
//                                                                                                 ^   ^^^^^^^^^^^^^^^^
//                                                                                            offset   suffix
//
// Without additional filters we extract a qualified type name e.g. "somelib::someclass::sometype<int>",
// if somebody wants to access unqualified name they can easily filter it based on ':' and '<' offsets.
//
// Strictly speaking we aren't even restricted to reflectable aggregates, the technique should work for all types.

struct compiler_specifics_type {
    constexpr static std::string_view example = mangled_type_name<int>();
    constexpr static std::size_t      offset  = example.find("int");
    constexpr static std::string_view suffix  = example.substr(offset + literal_size("int"));
};

template <class T>
constexpr std::string_view type_name = [] {
    constexpr std::string_view mangled = mangled_type_name<T>();

    constexpr std::size_t begin = compiler_specifics_type::offset;
    constexpr std::size_t end   = mangled.find(compiler_specifics_type::suffix);

    constexpr std::string_view qualified = mangled.substr(begin, end - begin);
    // qualified type name (e.g. 'lib::someclass:sometype'), on MSVC it may also be prefixed by "struct" / "class"

    constexpr std::size_t space_last  = qualified.find_last_of(' ');
    constexpr bool        space_found = space_last == std::string_view::npos;

    constexpr std::string_view normalized = space_found ? qualified : qualified.substr(space_last + 1);
    // qualified type name without compiler-specific prefixes

    return normalized;
}();

// Wrap for public API

template <class T>
constexpr std::string_view name = type_name<std::remove_cvref_t<T>>;

// =================
// --- Tuple API ---
// =================

// Tuple views can be quite easily constructed through fold expressions,
// they allow us to forward reflection into generic functions operating on tuples.

template <std::size_t... indices, reflectable T>
constexpr auto label_view_for_sequence(std::index_sequence<indices...>, T&&) noexcept {
    return std::array{label<indices, T>()...};
}

template <std::size_t... indices, reflectable T>
constexpr auto entry_view_for_sequence(std::index_sequence<indices...>, T&& structure) noexcept {
    return std::make_tuple(entry<indices>(std::forward<T>(structure))...);
}

// Wrap for public API

template <reflectable T>
[[nodiscard]] constexpr auto label_view(T&& = T{}) noexcept {
    return label_view_for_sequence(std::make_index_sequence<size<T>>{}, T{});
}

template <reflectable T>
[[nodiscard]] constexpr auto value_view(T&& structure) noexcept {
    return to_tie(std::forward<T>(structure));
}

template <reflectable T>
[[nodiscard]] constexpr auto entry_view(T&& structure) noexcept {
    return entry_view_for_sequence(std::make_index_sequence<size<T>>{}, std::forward<T>(structure));
}

// ==================
// --- Algorithms ---
// ==================

// Lowest-level algorithm we can implement is an indexed invoke,
// most other algorithms can be trivially implemented in its terms.
//
// Note the usage of 'std::conjunction' to require constraints on a parameter pack.

// clang-format off

template <std::size_t value>
using index_constant = std::integral_constant<std::size_t, value>;

template <std::size_t... indices, class F>
    requires std::conjunction_v<std::is_invocable<F, index_constant<indices>>...>
constexpr void for_sequence(std::index_sequence<indices...>, F&& f)
    noexcept(noexcept((std::invoke(f, index_constant<indices>{}), ...)))
{
    (std::invoke(f, index_constant<indices>{}), ...);
}

// Wrap for public API

template <reflectable T, class F>
constexpr void for_indices(F&& f)
    noexcept(noexcept(for_sequence(std::make_index_sequence<size<T>>{}, std::forward<F>(f))))
{
    for_sequence(std::make_index_sequence<size<T>>{}, std::forward<F>(f));
}

// clang-format on

} // namespace utl::reflect_struct::impl

// ______________________ PUBLIC API ______________________

namespace utl::reflect_struct {

using impl::reflectable;

using impl::size;
using impl::name;

using impl::label;
using impl::value;
using impl::entry;

using impl::label_view;
using impl::value_view;
using impl::entry_view;

using impl::for_indices;

} // namespace utl::reflect_struct

#endif
#endif // module utl::reflect_struct
