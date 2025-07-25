// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::bit
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_bit.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_BIT)
#ifndef UTLHEADERGUARD_BIT
#define UTLHEADERGUARD_BIT

#define UTL_BIT_VERSION_MAJOR 1
#define UTL_BIT_VERSION_MINOR 0
#define UTL_BIT_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <cassert>          // assert()
#include <climits>          // CHAR_BIT
#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits
#include <type_traits>      // enable_if_t<>, is_integral_v<>, is_enum_v<>

// ____________________ DEVELOPER DOCS ____________________

// With C++20 following functions will be added into std:
// - 'std::bit_width()' replaces 'bit::width()'
// - 'std::rotl()'      replaces 'bit::rotl()'
// - 'std::rotr()'      replaces 'bit::rotr()'
// the only difference is that std functions accept unsigned integers only,
// while 'bit::' accepts both signed & unsigned by treating signed as unsigned
// during bitwise operations, see notes below on why we can do it.
//
// Note that documented order of segments slightly differs from the actual
// implementation since we need to have some group operations defined upfront.

// ____________________ IMPLEMENTATION ____________________

namespace utl::bit::impl {

// Ensure target is two's complement, this includes pretty much every platform ever to
// the point that C++20 standardizes two's complement encoding as a requirement,
// this check exists purely to be pedantic and document our assumptions strictly
static_assert((-1 & 3) == 3);
// before C++20 following options could technically be the case:
//    1. (-1 & 3) == 1 => target is sign & magnitude encoded
//    2. (-1 & 3) == 2 => target is one's complement
//    3. (-1 & 3) == 3 => target is two's complement
// other integer encodings are not possible in the standard

// Note 1:
// The reason we specify two's complement encoding is because in it
// signed <-> unsigned casting preserves the bit pattern

// Note 2:
// Shifting negative numbers is technically considered UB, in practice every compiler implements
// signed bitshift as '(signed)( (unsigned)x << shift )' however they still act as if calling shift
// on a negative 'x < 0' is UB and therefore can never happen which can lead to weirdness with what
// compiler considers to be dead code elimination. This is why we do the casting explicitly and
// use custom 'lshift()' and 'rshift()' to avoid possible UB.
// see https://stackoverflow.com/a/29710927/28607141

// =============
// --- Utils ---
// =============

// --- SFINAE helpers ---
// ----------------------

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_enum = require<std::is_enum_v<T>>;

// --- Getters ---
// ---------------

constexpr std::size_t byte_size = CHAR_BIT;

template <class T>
constexpr std::size_t size_of = sizeof(T) * byte_size;

// ============================
// --- Group Bit Operations ---
// ============================

// Left shift,
// unlike regular '<<' works properly with negative values, see notes above
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T lshift(T value, std::size_t shift) noexcept {
    assert(shift < size_of<T>);
    return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) << shift);
}

// Right shift,
// unlike regular '>>' works properly with negative values, see notes above
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T rshift(T value, std::size_t shift) noexcept {
    assert(shift < size_of<T>);
    return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) >> shift);
}

// Circular left rotate,
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T rotl(T value, std::size_t shift) noexcept {
    assert(shift < size_of<T>);
    return lshift(value, shift) | rshift(value, std::numeric_limits<T>::digits - shift);
}

// Circular right rotate,
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T rotr(T value, std::size_t shift) noexcept {
    assert(shift < size_of<T>);
    return lshift(value, std::numeric_limits<T>::digits - shift) | rshift(value, shift);
}

// ================
// --- Counters ---
//  ===============

// Equivalent to C++20 'std::bit_width', but works with signed integers
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr std::size_t width(T value) noexcept {
    auto        uvalue = static_cast<std::make_unsigned_t<T>>(value);
    std::size_t count  = 0;
    while (uvalue) ++count, uvalue >>= 1;
    return count;
    // can be done faster if we write in a "nasty" way, see https://graphics.stanford.edu/~seander/bithacks.html
    // this isn't done because at the end of the day the truly fast way of doing it is though intrinsics directly,
    // better keep the non-intrinsic implementation clean & generic and hope that compiler realizes what we're doing
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr std::size_t popcount(T value) noexcept {
    constexpr auto bitmask_1 = T(0x5555555555555555UL);
    constexpr auto bitmask_2 = T(0x3333333333333333UL);
    constexpr auto bitmask_3 = T(0x0F0F0F0F0F0F0F0FUL);

    constexpr auto bitmask_16 = T(0x00FF00FF00FF00FFUL);
    constexpr auto bitmask_32 = T(0x0000FFFF0000FFFFUL);
    constexpr auto bitmask_64 = T(0x00000000FFFFFFFFUL);

    value = (value & bitmask_1) + (rshift(value, 1) & bitmask_1);
    value = (value & bitmask_2) + (rshift(value, 2) & bitmask_2);
    value = (value & bitmask_3) + (rshift(value, 4) & bitmask_3);

    if constexpr (sizeof(T) > 1) value = (value & bitmask_16) + (rshift(value, 8) & bitmask_16);
    if constexpr (sizeof(T) > 2) value = (value & bitmask_32) + (rshift(value, 16) & bitmask_32);
    if constexpr (sizeof(T) > 4) value = (value & bitmask_64) + (rshift(value, 32) & bitmask_64);

    return value;
    // GCC seems to be smart enough to replace this with a built-in
}

// =================================
// --- Individual Bit Operations ---
// =================================

// Get individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr bool get(T value, std::size_t bit) noexcept {
    assert(bit < size_of<T>);
    return rshift(value, bit) & T(1);
}

// Set individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
constexpr T set(T value, std::size_t bit) noexcept {
    assert(bit < size_of<T>);
    return value | lshift(T(1), bit);
}

// Clear individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
constexpr T clear(T value, std::size_t bit) noexcept {
    assert(bit < size_of<T>);
    return value & ~lshift(T(1), bit);
}

// Flip individual bits,
// undefined behavior if 'bit >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
constexpr T flip(T value, std::size_t bit) noexcept {
    assert(bit < size_of<T>);
    return value ^ lshift(T(1), bit);
}

// =====================
// --- Enum Bitflags ---
// =====================

template <class E, require_enum<E> = true>
[[nodiscard]] constexpr auto to_underlying(E value) noexcept {
    return static_cast<std::underlying_type_t<E>>(value); // in C++23 gets replaced by 'std::to_underlying()'
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr auto to_bool(T value) noexcept {
    return static_cast<bool>(value);
}

// Thin wrapper around an enum that gives it bitflag semantics
template <class E, require_enum<E> = true>
class Flags {
    std::underlying_type_t<E> data{};

    constexpr Flags(std::underlying_type_t<E> value) noexcept : data(value) {}

public:
    // clang-format off
    constexpr Flags(E flag) noexcept : data(to_underlying(flag)) {}
    constexpr Flags(std::initializer_list<E> flag_list) noexcept { for (auto flag : flag_list) this->add(flag); }
    
    constexpr operator bool() const noexcept { return to_bool(this->data); }
    
    [[nodiscard]] constexpr E get() const noexcept { return static_cast<E>(this->data); }

    [[nodiscard]] constexpr bool contains(E      flag) const noexcept { return to_bool(this->data & to_underlying(flag)); }
    [[nodiscard]] constexpr bool contains(Flags other) const noexcept { return to_bool(this->data & other.data         ); }

    constexpr Flags& add(E      flag) noexcept { this->data |= to_underlying(flag); return *this; }
    constexpr Flags& add(Flags other) noexcept { this->data |= other.data;          return *this; }

    constexpr Flags& remove(E      flag) noexcept { this->data &= ~to_underlying(flag); return *this; }
    constexpr Flags& remove(Flags other) noexcept { this->data &= ~other.data;          return *this; }

    [[nodiscard]] constexpr Flags operator~() const noexcept { return Flags{~this->data}; };
    
    [[nodiscard]] constexpr Flags operator|(Flags other) const noexcept { return Flags{this->data | other.data}; }
    [[nodiscard]] constexpr Flags operator&(Flags other) const noexcept { return Flags{this->data & other.data}; }
    
    constexpr Flags& operator|=(Flags other) noexcept { this->data |= other.data; return *this; }
    constexpr Flags& operator&=(Flags other) noexcept { this->data &= other.data; return *this; }
    
    [[nodiscard]] constexpr bool operator==(Flags other) noexcept { return this->data == other.data; }
    [[nodiscard]] constexpr bool operator!=(Flags other) noexcept { return this->data != other.data; }
    [[nodiscard]] constexpr bool operator<=(Flags other) noexcept { return this->data <= other.data; }
    [[nodiscard]] constexpr bool operator>=(Flags other) noexcept { return this->data >= other.data; }
    [[nodiscard]] constexpr bool operator< (Flags other) noexcept { return this->data <  other.data; }
    [[nodiscard]] constexpr bool operator> (Flags other) noexcept { return this->data >  other.data; }
    // clang-format on
};

} // namespace utl::bit::impl

// ______________________ PUBLIC API ______________________

namespace utl::bit {

using impl::get;
using impl::set;
using impl::clear;
using impl::flip;

using impl::lshift;
using impl::rshift;
using impl::rotl;
using impl::rotr;

using impl::byte_size;
using impl::size_of;

using impl::width;
using impl::popcount;

using impl::Flags;

} // namespace utl::bit

#endif
#endif // module utl::bit






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






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::integral
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_integral.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_INTEGRAL)
#ifndef UTLHEADERGUARD_INTEGRAL
#define UTLHEADERGUARD_INTEGRAL

#define UTL_INTEGRAL_VERSION_MAJOR 1
#define UTL_INTEGRAL_VERSION_MINOR 0
#define UTL_INTEGRAL_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <cassert>     // assert()
#include <climits>     // CHAR_BIT
#include <cstddef>     // size_t
#include <cstdint>     // uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
#include <limits>      // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <stdexcept>   // domain_error
#include <string>      // string, to_string()
#include <type_traits> // enable_if_t<>, is_integral_v<>, is_unsigned_v<>, make_unsigned_t<>

// ____________________ DEVELOPER DOCS ____________________

// With C++20 following functions will be added into 'std::':
//    - cmp_equal()
//    - cmp_not_equal()
//    - cmp_less()
//    - cmp_greater()
//    - cmp_less_equal()
//    - cmp_greater_equal()
//
// With C++26 following functions will be added into 'std::':
//    - add_sat()
//    - sub_sat()
//    - mul_sat()
//    - div_sat()
//    - saturate_cast()

// ____________________ IMPLEMENTATION ____________________

namespace utl::integral::impl {

// ======================
// --- SFINAE helpers ---
// ======================

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

using ull = unsigned long long;

// =================================
// --- Rounding integer division ---
// =================================

// Rounding integer division functions that can properly handle all signed
// values and don't run into overflow issues are surprisingly tricky to
// implement, most implementation found online are blatantly erroneous,
// some good details on the topic can be found in <intdiv> C++26 proposal,
// see https://gist.github.com/Eisenwave/2a7d7a4e74e99bbb513984107a6c63ef

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T div_floor(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    const bool quotient_negative = (dividend < T(0)) != (divisor < T(0));
    return dividend / divisor - (dividend % divisor != T(0) && quotient_negative);
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T div_ceil(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    const bool quotient_positive = (dividend < T(0)) == (divisor < T(0));
    return dividend / divisor + (dividend % divisor != T(0) && quotient_positive);
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T div_down(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    return dividend / divisor;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T div_up(T dividend, T divisor) noexcept {
    assert(divisor != T(0));

    const T quotient_sign = (dividend < T(0) ? T(-1) : T(1)) * (divisor < T(0) ? T(-1) : T(1));
    return dividend / divisor + (dividend % divisor != T(0)) * quotient_sign;
}

// ======================
// --- Saturated math ---
// ======================

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr bool add_overflows(T lhs, T rhs) noexcept {
    if (rhs > T(0) && lhs > std::numeric_limits<T>::max() - rhs) return false;
    if (rhs < T(0) && lhs < std::numeric_limits<T>::min() - rhs) return false;
    return true;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr bool sub_overflows(T lhs, T rhs) noexcept {
    if (rhs < T(0) && lhs > std::numeric_limits<T>::max() + rhs) return false;
    if (rhs > T(0) && lhs < std::numeric_limits<T>::min() + rhs) return false;
    return true;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr bool mul_overflows(T lhs, T rhs) noexcept {
    constexpr auto max = std::numeric_limits<T>::max();
    constexpr auto min = std::numeric_limits<T>::min();

    if (lhs < T(0) && rhs < T(0) && rhs < max / lhs) return true;
    if (lhs < T(0) && rhs > T(0) && lhs < min / rhs) return true;
    if (lhs > T(0) && rhs < T(0) && rhs < min / lhs) return true;
    if (lhs > T(0) && rhs > T(0) && lhs > max / rhs) return true;
    return false;

    // Note 1:
    // There is no portable way to implement truly performant saturated multiplication, C++26 standard
    // saturated functions are implemented in terms of '__builtin_mul_overflow' and '__mulh'
    // intrinsics which can speed this up quite significantly due to not having any division

    // Note 2:
    // We have to use different branches depending on the lhs/rhs signs and swap division order due to asymmetry
    // in signed integer range, for example, for 32-bit int 'min = -2147483648', while 'max = 2147483647',
    // -2147483648 * -1  =>  positive  =>  can overflow max  =>  mul overflows, 'max / rhs' overflows, 'max / lhs' fine
    // -2147483648 *  1  =>  negative  =>  can overflow min  =>  mul      fine, 'min / rhs'      fine, 'min / lhs' fine
    //  2147483647 * -1  =>  negative  =>  can overflow min  =>  mul      fine, 'min / rhs'      fine, 'min / lhs' fine
    //  2147483647 *  1  =>  positive  =>  can overflow max  =>  mul      fine, 'max / rhs'      fine, 'max / lhs' fine

    return false;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr bool div_overflows(T lhs, T rhs) noexcept {
    assert(rhs != T(0));

    // Unsigned division can't overflow
    if constexpr (std::is_unsigned_v<T>) return false;
    // Signed division overflows only for 'min / -1', this case is illustrated in 'mul_overflows()' comments
    else return lhs == std::numeric_limits<T>::min() && rhs == T(-1);
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T add_sat(T lhs, T rhs) noexcept {
    if (rhs > T(0) && lhs > std::numeric_limits<T>::max() - rhs) return std::numeric_limits<T>::max();
    if (rhs < T(0) && lhs < std::numeric_limits<T>::min() - rhs) return std::numeric_limits<T>::min();
    return lhs + rhs;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T sub_sat(T lhs, T rhs) noexcept {
    if (rhs < T(0) && lhs > std::numeric_limits<T>::max() + rhs) return std::numeric_limits<T>::max();
    if (rhs > T(0) && lhs < std::numeric_limits<T>::min() + rhs) return std::numeric_limits<T>::min();
    return lhs - rhs;
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T mul_sat(T lhs, T rhs) noexcept {
    constexpr auto max = std::numeric_limits<T>::max();
    constexpr auto min = std::numeric_limits<T>::min();

    if (lhs < 0 && rhs < 0 && rhs < max / lhs) return max;
    if (lhs < 0 && rhs > 0 && lhs < min / rhs) return min;
    if (lhs > 0 && rhs < 0 && rhs < min / lhs) return min;
    if (lhs > 0 && rhs > 0 && lhs > max / rhs) return max;
    return lhs * rhs;
} // see 'mul_overflows()' comments for a detailed explanation

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T div_sat(T lhs, T rhs) noexcept {
    assert(rhs != T(0));

    // Unsigned division can't overflow
    if constexpr (std::is_unsigned_v<T>) return lhs / rhs;
    // Signed division overflows only for 'min / -1', this case is illustrated in 'mul_overflows()' comments
    else return (lhs == std::numeric_limits<T>::min() && rhs == T(-1)) ? std::numeric_limits<T>::max() : lhs / rhs;
}

// =========================================
// --- Heterogeneous integer comparators ---
// =========================================

// Integer comparators that properly handle differently signed integers, becomes part of 'std' in C++20

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_equal(T1 lhs, T2 rhs) noexcept {
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>) return lhs == rhs;
    else if constexpr (std::is_signed_v<T1>) return lhs >= 0 && std::make_unsigned_t<T1>(lhs) == rhs;
    else return rhs >= 0 && std::make_unsigned_t<T2>(rhs) == lhs;
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_not_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_equal(lhs, rhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_less(T1 lhs, T2 rhs) noexcept {
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>) return lhs < rhs;
    else if constexpr (std::is_signed_v<T1>) return lhs < 0 || std::make_unsigned_t<T1>(lhs) < rhs;
    else return rhs >= 0 && lhs < std::make_unsigned_t<T2>(rhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_greater(T1 lhs, T2 rhs) noexcept {
    return cmp_less(rhs, lhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_less_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_less(rhs, lhs);
}

template <class T1, class T2>
[[nodiscard]] constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept {
    return !cmp_less(lhs, rhs);
}

// Returns if 'value' is in range of type 'To'
template <class To, class From>
[[nodiscard]] constexpr bool in_range(From value) noexcept {
    return cmp_greater_equal(value, std::numeric_limits<To>::min()) &&
           cmp_less_equal(value, std::numeric_limits<To>::max());
}

// =============
// --- Casts ---
// =============

// Integer-to-integer cast that throws if conversion would overflow/underflow the result,
// no '[[nodiscard]]' because cast may be used for the side effect of throwing
template <class To, class From, require_integral<To> = true, require_integral<From> = true>
constexpr To narrow_cast(From value) {
    if (!in_range<To>(value)) throw std::domain_error("narrow_cast() overflows the result.");
    return static_cast<To>(value);
}

template <class To, class From, require_integral<To> = true, require_integral<From> = true>
[[nodiscard]] constexpr To saturate_cast(From value) noexcept {
    constexpr auto to_min      = std::numeric_limits<To>::min();
    constexpr auto to_max      = std::numeric_limits<To>::max();
    constexpr int  to_digits   = std::numeric_limits<To>::digits;
    constexpr int  from_digits = std::numeric_limits<From>::digits;

    // signed -> signed
    if constexpr (std::is_signed_v<From> && std::is_signed_v<To>) {
        // value outside of type range => clamp to range
        if constexpr (to_digits < from_digits) {
            if (value < static_cast<From>(to_min)) return to_min;
            if (value > static_cast<From>(to_max)) return to_max;
        }
    }
    // signed -> unsigned
    else if constexpr (std::is_unsigned_v<To>) {
        // value negative => clamp to 0
        if (value < static_cast<From>(to_min)) return to_min;
        // value too big after casting => clamp to max
        // note that we rely on operator '>' being able to compare unsigned types of different sizes,
        // a more explicit way would be to compare 'std::common_type_t<std::make_unsigned_t<From>, To>,
        // but it doesn't really achieve anything except verbosity
        else if (std::make_unsigned_t<From>(value) > to_max) return to_max;
    }
    // unsigned -> signed
    else if constexpr (std::is_unsigned_v<From>) {
        // value too big => clamp to max
        // like before 'make_unsigned_t' is here to make both sides of comparison unsigned
        if (value > std::make_unsigned_t<To>(to_max)) return to_max;
    }

    // unsigned -> unsigned
    // + everything that didn't trigger a runtime saturating condition
    return static_cast<To>(value);
}

template <class T, require_integral<T> = true>
constexpr auto to_signed(T value) { // no '[[nodiscard]]' because cast may be used for the side effect of throwing
    return narrow_cast<std::make_signed_t<T>>(value);
}

template <class T, require_integral<T> = true>
constexpr auto to_unsigned(T value) { // no '[[nodiscard]]' because cast may be used for the side effect of throwing
    return narrow_cast<std::make_unsigned_t<T>>(value);
}

// ================
// --- Literals ---
// ================

namespace literals {

// Literals for all fixed-size and commonly used integer types, 'narrow_cast()'
// ensures there is no overflow during initialization from 'unsigned long long'
// clang-format off
[[nodiscard]] constexpr auto operator""_i8  (ull v) noexcept { return narrow_cast<std::int8_t   >(v); }
[[nodiscard]] constexpr auto operator""_u8  (ull v) noexcept { return narrow_cast<std::uint8_t  >(v); }
[[nodiscard]] constexpr auto operator""_i16 (ull v) noexcept { return narrow_cast<std::int16_t  >(v); }
[[nodiscard]] constexpr auto operator""_u16 (ull v) noexcept { return narrow_cast<std::uint16_t >(v); }
[[nodiscard]] constexpr auto operator""_i32 (ull v) noexcept { return narrow_cast<std::int32_t  >(v); }
[[nodiscard]] constexpr auto operator""_u32 (ull v) noexcept { return narrow_cast<std::uint32_t >(v); }
[[nodiscard]] constexpr auto operator""_i64 (ull v) noexcept { return narrow_cast<std::int64_t  >(v); }
[[nodiscard]] constexpr auto operator""_u64 (ull v) noexcept { return narrow_cast<std::uint64_t >(v); }
[[nodiscard]] constexpr auto operator""_sz  (ull v) noexcept { return narrow_cast<std::size_t   >(v); }
[[nodiscard]] constexpr auto operator""_ptrd(ull v) noexcept { return narrow_cast<std::ptrdiff_t>(v); }
// clang-format on

} // namespace literals

} // namespace utl::integral::impl

// ______________________ PUBLIC API ______________________

namespace utl::integral {

using impl::div_floor;
using impl::div_ceil;
using impl::div_down;
using impl::div_up;

using impl::add_overflows;
using impl::sub_overflows;
using impl::mul_overflows;
using impl::div_overflows;

using impl::add_sat;
using impl::sub_sat;
using impl::mul_sat;
using impl::div_sat;

using impl::cmp_equal;
using impl::cmp_not_equal;
using impl::cmp_less;
using impl::cmp_greater;
using impl::cmp_less_equal;
using impl::cmp_greater_equal;

using impl::in_range;

using impl::narrow_cast;
using impl::saturate_cast;

using impl::to_signed;
using impl::to_unsigned;

namespace literals = impl::literals;

} // namespace utl::integral

#endif
#endif // module utl::integral






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::json
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_json.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_JSON)
#ifndef UTLHEADERGUARD_JSON
#define UTLHEADERGUARD_JSON

#define UTL_JSON_VERSION_MAJOR 1
#define UTL_JSON_VERSION_MINOR 1
#define UTL_JSON_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <array>            // array<>, size_t
#include <charconv>         // to_chars(), from_chars(), errc
#include <cmath>            // isfinite()
#include <cstdint>          // uint8_t, uint16_t, uint32_t
#include <filesystem>       // create_directories()
#include <fstream>          // ifstream, ofstream
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::max_digits10, numeric_limits<>::max_exponent10
#include <map>              // map<>
#include <stdexcept>        // runtime_error
#include <string>           // string
#include <string_view>      // string_view
#include <type_traits>      // enable_if<>, is_convertible<>, is_same<>, conjunction<>, disjunction<>, negation<>, ...
#include <utility>          // move(), declval<>()
#include <variant>          // variant<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Reasonably simple (discounting reflection) DOM parser / serializer, doesn't use any intrinsics or compiler-specific
// stuff. Unlike some other implementations, doesn't include the tokenizing step - we parse everything in a single 1D
// scan over the data, constructing recursive JSON struct on the fly. The main reason we can do this so easily is
// due to a nice quirk of JSON: when parsing nodes, we can always determine node type based on a single first
// character, see 'Parser::parse_node()'.
//
// Struct reflection is implemented through macros - alternative way would be to use templates with __PRETTY_FUNCTION__
// (or __FUNCSIG__) and do some constexpr string parsing to perform "magic" reflection without requiring macros, but
// that relies on the implementation-defined format of those strings and adds quite a lot more complexity.
// 'nlohmann_json' provides similar macros but also has a way of specializing things manually.
//
// Proper type traits and 'if constexpr' recursive introspection are a key to making APIs that can convert stuff
// between JSON and other types seamlessly, which is exactly what we do here, it even accounts for reflection.

// ____________________ IMPLEMENTATION ____________________

// ======================================
// --- libc++ 'from_chars()' fallback ---
// ======================================

// Problem:
//
// libc++ is extremely behind on C++17 support and doesn't provide floating point 'std::from_chars()' until version 20
// (which was released in 2025), we need a thread-safe locale-neutral fallback for it. This is mostly a MacOS issue.
//
// All float parsing functions before <charconv> are locale-dependent, the only way to have thread safety is through a
// specific 'std::istringstream' usage with a locally imbued locale. This is extremely slow and gives up roundtrip, but
// this is necessary to support MacOS. A "proper" solution would be to include a <charconv> reimplementation, but doing
// this would ~quadruple the size of the library and introduce some licensing questions.

#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 200000
#define UTL_JSON_FROM_CHARS_FALLBACK
#endif

#ifdef UTL_JSON_FROM_CHARS_FALLBACK
#include <locale>  // locale::classic()
#include <sstream> // istringstream
#endif

namespace utl::json::impl {

// =====================
// --- Unicode stuff ---
// =====================

// Codepoint conversion function. We could use <codecvt> to do the same in a few lines,
// but <codecvt> was marked for deprecation in C++17 and fully removed in C++26, as of now
// there is no standard library replacement so we have to roll our own. This is likely to
// be more performant too due to not having any redundant locale handling.
//
// The function was tested for all valid codepoints (from U+0000 to U+10FFFF)
// against the <codecvt> implementation and proved to be exactly the same.
//
// Codepoint <-> UTF-8 conversion table (see https://en.wikipedia.org/wiki/UTF-8):
//
// | Codepoint range      | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
// |----------------------|----------|----------|----------|----------|
// | U+0000   to U+007F   | 0eeeffff |          |          |          |
// | U+0080   to U+07FF   | 110dddee | 10eeffff |          |          |
// | U+0800   to U+FFFF   | 1110cccc | 10ddddee | 10eeffff |          |
// | U+010000 to U+10FFFF | 11110abb | 10bbcccc | 10ddddee | 10eeffff |
//
// Characters 'a', 'b', 'c', 'd', 'e', 'f' correspond to the bits taken from the codepoint 'U+ABCDEF'
// (each letter in a codepoint is a hex corresponding to 4 bits, 6 positions => 24 bits of info).
// In terms of C++ 'U+ABCDEF' codepoints can be expressed as an integer hex-literal '0xABCDEF'.
//
inline bool codepoint_to_utf8(std::string& destination, std::uint32_t cp) {
    // returns success so we can handle the error message inside the parser itself

    std::array<char, 4> buffer;
    std::size_t         count;

    // 1-byte ASCII (codepoints U+0000 to U+007F)
    if (cp <= 0x007F) {
        buffer[0] = static_cast<char>(cp);
        count     = 1;
    }
    // 2-byte unicode (codepoints U+0080 to U+07FF)
    else if (cp <= 0x07FF) {
        buffer[0] = static_cast<char>(((cp >> 6) & 0x1F) | 0xC0);
        buffer[1] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
        count     = 2;
    }
    // 3-byte unicode (codepoints U+0800 to U+FFFF)
    else if (cp <= 0xFFFF) {
        buffer[0] = static_cast<char>(((cp >> 12) & 0x0F) | 0xE0);
        buffer[1] = static_cast<char>(((cp >> 6) & 0x3F) | 0x80);
        buffer[2] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
        count     = 3;
    }
    // 4-byte unicode (codepoints U+010000 to U+10FFFF)
    else if (cp <= 0x10FFFF) {
        buffer[0] = static_cast<char>(((cp >> 18) & 0x07) | 0xF0);
        buffer[1] = static_cast<char>(((cp >> 12) & 0x3F) | 0x80);
        buffer[2] = static_cast<char>(((cp >> 6) & 0x3F) | 0x80);
        buffer[3] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
        count     = 4;
    }
    // invalid codepoint
    else {
        return false;
    }

    destination.append(buffer.data(), count);
    return true;
}

// JSON '\u' escapes use UTF-16 surrogate pairs to encode codepoints outside of basic multilingual plane,
// see https://unicodebook.readthedocs.io/unicode_encodings.html
//     https://en.wikipedia.org/wiki/UTF-16
[[nodiscard]] constexpr std::uint32_t utf16_pair_to_codepoint(std::uint16_t high, std::uint16_t low) noexcept {
    return 0x10000 + ((high & 0x03FF) << 10) + (low & 0x03FF);
}

[[nodiscard]] inline std::string utf8_replace_non_ascii(std::string str, char replacement_char) noexcept {
    for (auto& e : str)
        if (static_cast<std::uint8_t>(e) > 127) e = replacement_char;
    return str;
}

// This seems the to be the fastest way of reading a text file
// into 'std::string' without invoking OS-specific methods
// See this StackOverflow thread:
// https://stackoverflow.com/questions/32169936/optimal-way-of-reading-a-complete-file-to-a-string-using-fstream
// And attached benchmarks:
// https://github.com/Sqeaky/CppFileToStringExperiments
[[nodiscard]] inline std::string read_file_to_string(const std::string& path) {
    using namespace std::string_literals;

    std::ifstream file(path, std::ios::ate | std::ios::binary); // open file and immediately seek to the end
    // opening file as binary allows us to skip pointless newline re-encoding
    if (!file.good()) throw std::runtime_error("Could not open file {"s + path + "."s);

    const auto file_size = file.tellg(); // returns cursor pos, which is the end of file
    file.seekg(std::ios::beg);           // seek to the beginning
    std::string chars(file_size, 0);     // allocate string of appropriate size
    file.read(chars.data(), file_size);  // read into the string
    return chars;
}

template <class T>
[[nodiscard]] constexpr int log10_ceil(T num) noexcept {
    return num < 10 ? 1 : 1 + log10_ceil(num / 10);
}

[[nodiscard]] inline std::string pretty_error(std::size_t cursor, const std::string& chars) {
    // Special case for empty buffers
    if (chars.empty()) return "";

    // "Normalize" cursor if it's at the end of the buffer
    if (cursor >= chars.size()) cursor = chars.size() - 1;

    // Get JSON line number
    std::size_t line_number = 1; // don't want to include <algorithm> just for a single std::count()
    for (std::size_t pos = 0; pos < cursor; ++pos)
        if (chars[pos] == '\n') ++line_number;

    // Get contents of the current line
    constexpr std::size_t max_left_width  = 24;
    constexpr std::size_t max_right_width = 24;

    std::size_t line_start = cursor;
    for (; line_start > 0; --line_start)
        if (chars[line_start - 1] == '\n' || cursor - line_start >= max_left_width) break;

    std::size_t line_end = cursor;
    for (; line_end < chars.size() - 1; ++line_end)
        if (chars[line_end + 1] == '\n' || line_end - cursor >= max_right_width) break;

    const std::string_view line_contents(chars.data() + line_start, line_end - line_start + 1);

    // Format output
    const std::string line_prefix =
        "Line " + std::to_string(line_number) + ": "; // fits into SSO buffer in almost all cases

    std::string res;
    res.reserve(7 + 2 * line_prefix.size() + 2 * line_contents.size());

    res += '\n';
    res += line_prefix;
    res += utf8_replace_non_ascii(std::string(line_contents), '?');
    res += '\n';
    res.append(line_prefix.size(), ' ');
    res.append(cursor - line_start, '-');
    res += '^';
    res.append(line_end - cursor, '-');
    res += " [!]";

    // Note:
    // To properly align cursor in the error message we would need to count "visible characters" in a UTF-8
    // string, properly iterating over grapheme clusters is a very complex task, usually done by a dedicated
    // library. We could just count codepoints, but that wouldn't account for combining characters. To prevent
    // error message from being misaligned we can just replace all non-ascii symbols with '?', this way errors
    // might be less pretty, but they will reliably show the true location of the error.

    return res;
}

// ===================
// --- Type traits ---
// ===================

#define utl_json_define_trait(trait_name_, ...)                                                                        \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

utl_json_define_trait(has_begin, std::declval<std::decay_t<T>>().begin());
utl_json_define_trait(has_end, std::declval<std::decay_t<T>>().end());
utl_json_define_trait(has_input_it, std::next(std::declval<T>().begin()));

utl_json_define_trait(has_key_type, std::declval<typename std::decay_t<T>::key_type>());
utl_json_define_trait(has_mapped_type, std::declval<typename std::decay_t<T>::mapped_type>());

#undef utl_json_define_trait

// Workaround for 'static_assert(false)' making program ill-formed even when placed inside an 'if constexpr'
// branch that never compiles. 'static_assert(always_false_v<T)' on the other hand doesn't,
// which means we can use it to mark branches that should never compile.
template <class>
constexpr bool always_false_v = false;

// =================
// --- Map-macro ---
// =================

// This is an implementation of a classic map-macro that applies function-macro to all
// elements of __VA_ARGS__, it looks much uglier than usual because we have to prefix
// everything with verbose 'utl_json_', but that's the price of avoiding name collisions.
//
// Created by William Swanson in 2012 and declared as public domain.
//
// Macro supports up to 365 arguments. We will need it for structure reflection.

#define utl_json_eval_0(...) __VA_ARGS__
#define utl_json_eval_1(...) utl_json_eval_0(utl_json_eval_0(utl_json_eval_0(__VA_ARGS__)))
#define utl_json_eval_2(...) utl_json_eval_1(utl_json_eval_1(utl_json_eval_1(__VA_ARGS__)))
#define utl_json_eval_3(...) utl_json_eval_2(utl_json_eval_2(utl_json_eval_2(__VA_ARGS__)))
#define utl_json_eval_4(...) utl_json_eval_3(utl_json_eval_3(utl_json_eval_3(__VA_ARGS__)))
#define utl_json_eval(...) utl_json_eval_4(utl_json_eval_4(utl_json_eval_4(__VA_ARGS__)))

#define utl_json_map_end(...)
#define utl_json_map_out
#define utl_json_map_comma ,

#define utl_json_map_get_end_2() 0, utl_json_map_end
#define utl_json_map_get_end_1(...) utl_json_map_get_end_2
#define utl_json_map_get_end(...) utl_json_map_get_end_1
#define utl_json_map_next_0(test, next, ...) next utl_json_map_out
#define utl_json_map_next_1(test, next) utl_json_map_next_0(test, next, 0)
#define utl_json_map_next(test, next) utl_json_map_next_1(utl_json_map_get_end test, next)

#define utl_json_map_0(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_1)(f, peek, __VA_ARGS__)
#define utl_json_map_1(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_0)(f, peek, __VA_ARGS__)

// Resulting macro, applies the function macro 'f' to each of the remaining parameters
#define utl_json_map(f, ...)                                                                                           \
    utl_json_eval(utl_json_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0)) static_assert(true)

// ===================================
// --- JSON type conversion traits ---
// ===================================

template <class T>
using object_type_impl = std::map<std::string, T, std::less<>>;
// 'std::less<>' makes map transparent, which means we can use 'find()' for 'std::string_view' keys
template <class T>
using array_type_impl  = std::vector<T>;
using string_type_impl = std::string;
using number_type_impl = double;
using bool_type_impl   = bool;
struct null_type_impl {
    [[nodiscard]] bool operator==(const null_type_impl&) const noexcept {
        return true;
    } // so we can check 'Null == Null'
};

// Note:
// It is critical that 'object_type_impl' can be instantiated with incomplete type 'T'.
// This allows us to declare recursive classes like this:
//
//    'struct Recursive { std::map<std::string, Recursive> data; }'
//
// Technically, there is nothing stopping any dynamically allocated container from supporting
// incomplete types, since dynamic allocation inherently means pointer indirection at some point,
// which makes 'sizeof(Container)' independent of 'T'.
//
// This requirement was only standardized for 'std::vector' and 'std::list' due to ABI breaking concerns.
// 'std::map' is not required to support incomplete types by the standard, however in practice it does support them
// on all compilers that I know of. Several other JSON libraries seem to rely on the same behaviour without any issues.
// The same cannot be said about 'std::unordered_map', which is why we don't use it.
//
// We could make a more pedantic choice and add a redundant level of indirection, but that both complicates
// implementation needlessly and reduces performance. A perfect solution would be to write our own map implementation
// tailored for JSON use cases and providing explicit support for heterogeneous lookup and incomplete types, but that
// alone would be grander in scale than this entire parser for a mostly non-critical benefit.

struct dummy_type {};

// 'possible_value_type<T>::value' evaluates to:
//    - 'T::value_type' if 'T' has 'value_type'
//    - 'dummy_type' otherwise
template <class T, class = void>
struct possible_value_type {
    using type = dummy_type;
};

template <class T>
struct possible_value_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::value_type>())>> {
    using type = typename T::value_type;
};

// 'possible_mapped_type<T>::value' evaluates to:
//    - 'T::mapped_type' if 'T' has 'mapped_type'
//    - 'dummy_type' otherwise
template <class T, class = void>
struct possible_mapped_type {
    using type = dummy_type;
};

template <class T>
struct possible_mapped_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::mapped_type>())>> {
    using type = typename T::mapped_type;
};
// these type traits are a key to checking properties of 'T::value_type' & 'T::mapped_type' for a 'T' which may
// or may not have them (which is exactly the case with recursive traits that we're going to use later to deduce
// convertibility to recursive JSON). 'dummy_type' here is necessary to end the recursion of 'std::disjunction'

#define utl_json_type_trait_conjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::conjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

#define utl_json_type_trait_disjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::disjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

// Note:
// The reason we use 'struct trait_name : std::conjunction<...>' instead of 'using trait_name = std::conjunction<...>'
// is because 1st option allows for recursive type traits, while 'using' syntax doesn't. We have some recursive type
// traits here in form of 'is_json_type_convertible<>', which expands over the 'T' checking that 'T', 'T::value_type'
// (if exists), 'T::mapped_type' (if exists) and their other layered value/mapped types are all satisfying the
// necessary convertibility trait. This allows us to make a trait which fully deduces whether some
// complex datatype can be converted to a JSON recursively.

utl_json_type_trait_conjunction(is_object_like, has_begin<T>, has_end<T>, has_key_type<T>, has_mapped_type<T>);
utl_json_type_trait_conjunction(is_array_like, has_begin<T>, has_end<T>, has_input_it<T>);
utl_json_type_trait_conjunction(is_string_like, std::is_convertible<T, std::string_view>);
utl_json_type_trait_conjunction(is_numeric_like, std::is_convertible<T, number_type_impl>);
utl_json_type_trait_conjunction(is_bool_like, std::is_same<T, bool_type_impl>);
utl_json_type_trait_conjunction(is_null_like, std::is_same<T, null_type_impl>);

utl_json_type_trait_disjunction(is_directly_json_convertible, is_string_like<T>, is_numeric_like<T>, is_bool_like<T>,
                                is_null_like<T>);

utl_json_type_trait_conjunction(
    is_json_convertible,
    std::disjunction<
        // either the type itself is convertible
        is_directly_json_convertible<T>,
        // ... or it's an array of convertible elements
        std::conjunction<is_array_like<T>, is_json_convertible<typename possible_value_type<T>::type>>,
        // ... or it's an object of convertible elements
        std::conjunction<is_object_like<T>, is_json_convertible<typename possible_mapped_type<T>::type>>>,
    // end recursion by short-circuiting conjunction with 'false' once we arrive to '_dummy_type',
    // arriving here means the type isn't convertible to JSON
    std::negation<std::is_same<T, dummy_type>>);

#undef utl_json_type_trait_conjunction
#undef utl_json_type_trait_disjunction

// ==================
// --- Node class ---
// ==================

enum class Format : std::uint8_t { PRETTY, MINIMIZED };

class Node;
inline void serialize_json_to_buffer(std::string& chars, const Node& node, Format format);

class Node {
public:
    using object_type = object_type_impl<Node>;
    using array_type  = array_type_impl<Node>;
    using string_type = string_type_impl;
    using number_type = number_type_impl;
    using bool_type   = bool_type_impl;
    using null_type   = null_type_impl;

private:
    using variant_type = std::variant<null_type, object_type, array_type, string_type, number_type, bool_type>;
    // 'null_type' should go first to ensure default-initialization creates 'null' nodes

    variant_type data{};

public:
    // -- Getters --
    // -------------

    template <class T>
    [[nodiscard]] T& get() {
        return std::get<T>(this->data);
    }

    template <class T>
    [[nodiscard]] const T& get() const {
        return std::get<T>(this->data);
    }

    [[nodiscard]] object_type& get_object() { return this->get<object_type>(); }
    [[nodiscard]] array_type&  get_array() { return this->get<array_type>(); }
    [[nodiscard]] string_type& get_string() { return this->get<string_type>(); }
    [[nodiscard]] number_type& get_number() { return this->get<number_type>(); }
    [[nodiscard]] bool_type&   get_bool() { return this->get<bool_type>(); }
    [[nodiscard]] null_type&   get_null() { return this->get<null_type>(); }

    [[nodiscard]] const object_type& get_object() const { return this->get<object_type>(); }
    [[nodiscard]] const array_type&  get_array() const { return this->get<array_type>(); }
    [[nodiscard]] const string_type& get_string() const { return this->get<string_type>(); }
    [[nodiscard]] const number_type& get_number() const { return this->get<number_type>(); }
    [[nodiscard]] const bool_type&   get_bool() const { return this->get<bool_type>(); }
    [[nodiscard]] const null_type&   get_null() const { return this->get<null_type>(); }

    template <class T>
    [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<T>(this->data);
    }

    [[nodiscard]] bool is_object() const noexcept { return this->is<object_type>(); }
    [[nodiscard]] bool is_array() const noexcept { return this->is<array_type>(); }
    [[nodiscard]] bool is_string() const noexcept { return this->is<string_type>(); }
    [[nodiscard]] bool is_number() const noexcept { return this->is<number_type>(); }
    [[nodiscard]] bool is_bool() const noexcept { return this->is<bool_type>(); }
    [[nodiscard]] bool is_null() const noexcept { return this->is<null_type>(); }

    template <class T>
    [[nodiscard]] T* get_if() noexcept {
        return std::get_if<T>(&this->data);
    }

    template <class T>
    [[nodiscard]] const T* get_if() const noexcept {
        return std::get_if<T>(&this->data);
    }

    // -- Object methods ---
    // ---------------------

    Node& operator[](std::string_view key) {
        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        if (this->is_null()) this->data = object_type{}; // 'null' converts to objects automatically
        auto& object = this->get_object();
        auto  it     = object.find(key);
        if (it == object.end()) it = object.emplace(key, Node{}).first;
        return it->second;
    }

    [[nodiscard]] const Node& operator[](std::string_view key) const {
        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        const auto& object = this->get_object();
        const auto  it     = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }

    [[nodiscard]] Node& at(std::string_view key) {
        // Non-const 'operator[]' inserts non-existent keys, '.at()' should throw instead
        auto&      object = this->get_object();
        const auto it     = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }

    [[nodiscard]] const Node& at(std::string_view key) const { return this->operator[](key); }

    [[nodiscard]] bool contains(std::string_view key) const {
        const auto& object = this->get_object();
        const auto  it     = object.find(std::string(key));
        return it != object.end();
    }

    template <class T>
    [[nodiscard]] const T& value_or(std::string_view key, const T& else_value) const {
        const auto& object = this->get_object();
        const auto  it     = object.find(std::string(key));
        if (it != object.end()) return it->second.get<T>();
        return else_value;
        // same thing as 'this->contains(key) ? json.at(key).get<T>() : else_value' but without a second map lookup
    }

    // -- Array methods ---
    // --------------------

    [[nodiscard]] Node& operator[](std::size_t pos) { return this->get_array()[pos]; }

    [[nodiscard]] const Node& operator[](std::size_t pos) const { return this->get_array()[pos]; }

    [[nodiscard]] Node& at(std::size_t pos) { return this->get_array().at(pos); }

    [[nodiscard]] const Node& at(std::size_t pos) const { return this->get_array().at(pos); }

    void push_back(const Node& node) {
        if (this->is_null()) this->data = array_type{}; // 'null' converts to arrays automatically
        this->get_array().push_back(node);
    }

    void push_back(Node&& node) {
        if (this->is_null()) this->data = array_type{}; // 'null' converts to arrays automatically
        this->get_array().push_back(node);
    }

    // -- Assignment --
    // ----------------

    // Converting assignment
    template <class T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node> &&
                                            !std::is_same_v<std::decay_t<T>, object_type> &&
                                            !std::is_same_v<std::decay_t<T>, array_type> &&
                                            !std::is_same_v<std::decay_t<T>, string_type> && is_json_convertible_v<T>,
                                        bool> = true>
    Node& operator=(const T& value) {
        // Don't take types that decay to Node/object/array/string to prevent
        // shadowing native copy/move assignment for those types

        // Several "type-like' characteristics can be true at the same time,
        // to resolve ambiguity we assign the following conversion priority:
        // string > object > array > bool > null > numeric

        if constexpr (is_string_like_v<T>) {
            this->data.emplace<string_type>(value);
        } else if constexpr (is_object_like_v<T>) {
            this->data.emplace<object_type>();
            auto& object = this->get_object();
            for (const auto& [key, val] : value) object[key] = val;
        } else if constexpr (is_array_like_v<T>) {
            this->data.emplace<array_type>();
            auto& array = this->get_array();
            for (const auto& elem : value) array.emplace_back(elem);
        } else if constexpr (is_bool_like_v<T>) {
            this->data.emplace<bool_type>(value);
        } else if constexpr (is_null_like_v<T>) {
            this->data.emplace<null_type>(value);
        } else if constexpr (is_numeric_like_v<T>) {
            this->data.emplace<number_type>(static_cast<number_type>(value));
            // cast silences possible 'size_t' -> 'double' conversion warnings,
            // conversion is deliberate and documented even if it might lose the upper 9 bits
        } else {
            static_assert(always_false_v<T>, "Method is a non-exhaustive visitor of std::variant<>.");
        }

        return *this;
    }

    // "native" copy/move semantics for types that support it
    Node& operator=(const object_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(object_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    Node& operator=(const array_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(array_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    Node& operator=(const string_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(string_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    // Support for 'std::initializer_list' type deduction,
    // (otherwise the call is ambiguous)
    template <class T>
    Node& operator=(std::initializer_list<T> ilist) {
        // We can't just do 'return *this = array_type(value);' because compiler doesn't realize it can
        // convert 'std::initializer_list<T>' to 'std::vector<Node>' for all 'T' convertible to 'Node',
        // we have to invoke 'Node()' constructor explicitly (here it happens in 'emplace_back()')
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) array_value.emplace_back(e);
        this->data = std::move(array_value);
        return *this;
    }

    template <class T>
    Node& operator=(std::initializer_list<std::initializer_list<T>> ilist) {
        // Support for 2D brace initialization
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) {
            array_value.emplace_back();
            array_value.back() = e;
        }
        // uses 1D 'operator=(std::initializer_list<T>)' to fill each node of the array
        this->data = std::move(array_value);
        return *this;
    }

    template <class T>
    Node& operator=(std::initializer_list<std::initializer_list<std::initializer_list<T>>> ilist) {
        // Support for 3D brace initialization
        // it's dumb, but it works
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) {
            array_value.emplace_back();
            array_value.back() = e;
        }
        // uses 2D 'operator=(std::initializer_list<std::initializer_list<T>>)' to fill each node of the array
        this->data = std::move(array_value);
        return *this;
    }

    // we assume no reasonable person would want to type a 4D+ array as 'std::initializer_list<>',
    // if they really want to they can specify the type of the top layer and still be fine

    // -- Constructors --
    // ------------------

    Node& operator=(const Node&) = default;
    Node& operator=(Node&&)      = default;

    Node()            = default;
    Node(const Node&) = default;
    Node(Node&&)      = default;
    // Note:
    // We suffer a lot if 'object_type' move-constructor is not marked 'noexcept', if that's the case
    // 'Node' move-constructor doesn't get 'noexcept' either which means `std::vector<Node>` will copy
    // nodes instead of moving when it grows. 'std::map' is NOT required to be noexcept by the standard
    // but it is marked as such in both 'libc++' and 'libstdc++', 'VS' stdlib lacks behind in that regard.
    // See noexcept status summary here: http://howardhinnant.github.io/container_summary.html

    // Converting ctor
    template <class T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node> &&
                                            !std::is_same_v<std::decay_t<T>, object_type> &&
                                            !std::is_same_v<std::decay_t<T>, array_type> &&
                                            !std::is_same_v<std::decay_t<T>, string_type> && is_json_convertible_v<T>,
                                        bool> = true>
    Node(const T& value) {
        *this = value;
    }

    Node(const object_type& value) { this->data = value; }
    Node(object_type&& value) { this->data = std::move(value); }
    Node(const array_type& value) { this->data = value; }
    Node(array_type&& value) { this->data = std::move(value); }
    Node(std::string_view value) { this->data = string_type(value); }
    Node(const string_type& value) { this->data = value; }
    Node(string_type&& value) { this->data = std::move(value); }
    Node(number_type value) { this->data = value; }
    Node(bool_type value) { this->data = value; }
    Node(null_type value) { this->data = value; }

    // --- JSON Serializing public API ---
    // -----------------------------------

    [[nodiscard]] std::string to_string(Format format = Format::PRETTY) const {
        std::string buffer;
        serialize_json_to_buffer(buffer, *this, format);
        return buffer;
    }

    void to_file(const std::string& filepath, Format format = Format::PRETTY) const {
        const auto chars = this->to_string(format);

        const std::filesystem::path path = filepath;
        if (path.has_parent_path() && !std::filesystem::exists(path.parent_path()))
            std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
        // no need to do an OS call in a trivial case, some systems might also have limited permissions
        // on directory creation and calling 'create_directories()' straight up will cause them to error
        // even when there is no need to actually perform directory creation because it already exists

        // if user doesn't want to pay for 'create_directories()' call (which seems to be inconsequential
        // on my benchmarks) they can always use 'std::ofstream' and 'to_string()' to export manually

        std::ofstream(filepath).write(chars.data(), chars.size());
        // maybe a little faster than doing 'std::ofstream(filepath) << node.to_string(format)'
    }

    // --- Reflection ---
    // ------------------

    template <class T>
    [[nodiscard]] T to_struct() const {
        static_assert(
            always_false_v<T>,
            "Provided type doesn't have a defined JSON reflection. Use 'UTL_JSON_REFLECT' macro to define one.");
        // compile-time protection against calling 'to_struct()' on types that don't have reflection,
        // we can also provide a proper error message here
        return {};
        // this is needed to silence "no return in a function" warning that appears even if this specialization
        // (which by itself should cause a compile error) doesn't get compiled

        // specializations of this template that will actually perform the conversion will be defined by
        // macros outside the class body, this is a perfectly legal thing to do, even if unintuitive compared
        // to non-template members, see https://en.cppreference.com/w/cpp/language/member_template
    }
};

// Public typedefs
using Object = Node::object_type;
using Array  = Node::array_type;
using String = Node::string_type;
using Number = Node::number_type;
using Bool   = Node::bool_type;
using Null   = Node::null_type;

// =====================
// --- Lookup Tables ---
// =====================

constexpr std::uint8_t u8(char value) { return static_cast<std::uint8_t>(value); }

constexpr std::size_t number_of_char_values = 256;

// Lookup table used to check if number should be escaped and get a replacement char at the same time.
// This allows us to replace multiple checks and if's with a single array lookup.
//
// Instead of:
//    if (c == '"') { chars += '"' }
//    ...
//    else if (c == '\t') { chars += 't' }
// we get:
//    if (const char replacement = lookup_serialized_escaped_chars[u8(c)]) { chars += replacement; }
//
// which ends up being a bit faster and also nicer.
//
// Note:
// It is important that we explicitly cast to 'uint8_t' when indexing, depending on the platform 'char' might
// be either signed or unsigned, we don't want our array to be indexed at '-71'. While we can reasonably expect
// ASCII encoding on the platform (which would put all char literals that we use into the 0-127 range) other chars
// might still be negative. This shouldn't have any cost as trivial int casts like these involve no runtime logic.
//
constexpr std::array<char, number_of_char_values> lookup_serialized_escaped_chars = [] {
    std::array<char, number_of_char_values> res{};
    // default-initialized chars get initialized to '\0',
    // ('\0' == 0) is mandated by the standard, which is why we can use it inside an 'if' condition
    res[u8('"')]  = '"';
    res[u8('\\')] = '\\';
    // res[u8('/')]  = '/'; escaping forward slash in JSON is allowed, but redundant
    res[u8('\b')] = 'b';
    res[u8('\f')] = 'f';
    res[u8('\n')] = 'n';
    res[u8('\r')] = 'r';
    res[u8('\t')] = 't';
    return res;
}();

// Lookup table used to determine "insignificant whitespace" characters when
// skipping whitespace during parser. Seems to be either similar or marginally
// faster in performance than a regular condition check.
//
// "Insignificant whitespace" according to the JSON spec:
//    https://ecma-international.org/wp-content/uploads/ECMA-404.pdf
//
// constitutes following symbols:
//    - Whitespace      (aka ' ' )
//    - Tabs            (aka '\t')
//    - Carriage return (aka '\r')
//    - Newline         (aka '\n')
//
constexpr std::array<bool, number_of_char_values> lookup_whitespace_chars = [] {
    std::array<bool, number_of_char_values> res{};
    res[u8(' ')]  = true;
    res[u8('\t')] = true;
    res[u8('\r')] = true;
    res[u8('\n')] = true;
    return res;
}();

// Lookup table used to get an appropriate char for the escaped char in a 2-char JSON escape sequence.
constexpr std::array<char, number_of_char_values> lookup_parsed_escaped_chars = [] {
    std::array<char, number_of_char_values> res{};
    res[u8('"')]  = '"';
    res[u8('\\')] = '\\';
    res[u8('/')]  = '/';
    res[u8('b')]  = '\b';
    res[u8('f')]  = '\f';
    res[u8('n')]  = '\n';
    res[u8('r')]  = '\r';
    res[u8('t')]  = '\t';
    return res;
}();

// ======================================
// --- libc++ 'from_chars()' fallback ---
// ======================================

#ifdef UTL_JSON_FROM_CHARS_FALLBACK
constexpr std::array<bool, number_of_char_values> lookup_numeric_chars = [] {
    std::array<bool, number_of_char_values> res{};
    for (std::uint8_t i = u8('0'); i <= u8('9'); ++i) res[i] = true;
    res[u8('-')] = true;
    res[u8('+')] = true;
    res[u8('e')] = true;
    res[u8('E')] = true;
    res[u8('.')] = true;
    return res;
}();

// Basically the only thread-safe locale-neutral way of parsing floats that doesn't involve a hand-rolled <charconv>
// (which takes approx. 4k lines of rather terse code), VERY slow and doesn't provide <charconv> roundtrip guarantees.
//
// 'stod()' and 'strtod()' are inherently not thread-safe due to their dependence on a global locale with no thread
// safety guarantees, many smaller JSON libs fumble this aspect since it's very easy to miss and produces bugs that
// are difficult to pinpoint. Float parsing situation is rather grim before C++17.
//
// For usage convenience we expose the same API as regular 'from_chars()'.
//
inline std::from_chars_result available_from_chars_impl(const char* first, const char* last, Number& value) {
    const char* cursor = first;
    while (cursor < last && lookup_numeric_chars[u8(*cursor)]) ++cursor; // skip to the first non-numeric char

    std::istringstream iss(std::string(first, cursor));
    iss.imbue(std::locale::classic());
    iss >> value;

    const auto error = (iss && iss.eof()) ? std::errc{} : std::errc::invalid_argument;
    // '.eof()' not set => number wasn't parsed fully (usually due to incorrect format)
    // 'iss' is false   => number parsing encountered an algorithmic issue
    if (error != std::errc{}) return {first, error};
    return {cursor, error};
}
#else
inline std::from_chars_result available_from_chars_impl(const char* first, const char* last, Number& value) {
    return std::from_chars(first, last, value);
}
#endif

// ==========================
// --- JSON Parsing impl. ---
// ==========================

constexpr unsigned int default_recursion_limit = 100;
// this recursion limit applies only to parsing from text, conversions from
// structs & containers are a separate thing and don't really need it as much

struct Parser {
    const std::string& chars;
    unsigned int       recursion_limit;
    unsigned int       recursion_depth = 0;
    // we track recursion depth to handle stack allocation errors
    // (this can be caused malicious inputs with extreme level of nesting, for example, 100k array
    // opening brackets, which would cause huge recursion depth causing the stack to overflow with SIGSEGV)

    // dynamic allocation errors can be handled with regular exceptions through std::bad_alloc

    Parser() = delete;
    Parser(const std::string& chars, unsigned int& recursion_limit) : chars(chars), recursion_limit(recursion_limit) {}

    std::size_t skip_nonsignificant_whitespace(std::size_t cursor) {
        using namespace std::string_literals;

        while (cursor < this->chars.size()) {
            if (!lookup_whitespace_chars[u8(this->chars[cursor])]) return cursor;
            ++cursor;
        }

        throw std::runtime_error("JSON parser reached the end of buffer at pos "s + std::to_string(cursor) +
                                 " while skipping insignificant whitespace segment."s +
                                 pretty_error(cursor, this->chars));
    }

    std::pair<std::size_t, Node> parse_node(std::size_t cursor) {
        using namespace std::string_literals;

        // Node selector assumes it is starting at a significant symbol
        // which is the first symbol of the node to be parsed

        const char c = this->chars[cursor];

        // Assuming valid JSON, we can determine node type based on a single first character
        if (c == '{') {
            return this->parse_object(cursor);
        } else if (c == '[') {
            return this->parse_array(cursor);
        } else if (c == '"') {
            return this->parse_string(cursor);
        } else if (('0' <= c && c <= '9') || (c == '-')) {
            return this->parse_number(cursor);
        } else if (c == 't') {
            return this->parse_true(cursor);
        } else if (c == 'f') {
            return this->parse_false(cursor);
        } else if (c == 'n') {
            return this->parse_null(cursor);
        }
        throw std::runtime_error("JSON node selector encountered unexpected marker symbol {"s + this->chars[cursor] +
                                 "} at pos "s + std::to_string(cursor) + " (should be one of {0123456789{[\"tfn})."s +
                                 pretty_error(cursor, this->chars));

        // Note: using a lookup table instead of an 'if' chain doesn't seem to offer any performance benefits here
    }

    std::size_t parse_object_pair(std::size_t cursor, Object& parent) {
        using namespace std::string_literals;

        // Object pair parser assumes it is starting at a '"'

        // Parse pair key
        std::string key; // allocating a string here is fine since we will 'std::move()' it into a map key
        std::tie(cursor, key) = this->parse_string(cursor);

        // Handle stuff in-between
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ':')
            throw std::runtime_error("JSON object node encountered unexpected symbol {"s + this->chars[cursor] +
                                     "} after the pair key at pos "s + std::to_string(cursor) + " (should be {:})."s +
                                     pretty_error(cursor, this->chars));
        ++cursor; // move past the colon ':'
        cursor = this->skip_nonsignificant_whitespace(cursor);

        // Parse pair value
        if (++this->recursion_depth > this->recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                                     std::to_string(this->recursion_limit) +
                                     ". If stated depth wasn't caused by an invalid input, "s +
                                     "recursion limit can be increased in the parser."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        // Note 1:
        // The question of whether JSON allows duplicate keys is non-trivial but the resulting answer is YES.
        // JSON is governed by 2 standards:
        // 1) ECMA-404 https://ecma-international.org/wp-content/uploads/ECMA-404.pdf
        //    which doesn't say anything about duplicate kys
        // 2) RFC-8259 https://www.rfc-editor.org/rfc/rfc8259
        //    which states "The names within an object SHOULD be unique.",
        //    however as defined in RFC-2119 https://www.rfc-editor.org/rfc/rfc2119:
        //       "SHOULD This word, or the adjective "RECOMMENDED", mean that there may exist valid reasons in
        //       particular circumstances to ignore a particular item, but the full implications must be understood
        //       and carefully weighed before choosing a different course."
        // which means at the end of the day duplicate keys are discouraged, but still valid

        // Note 2:
        // There is no standard specification on which JSON value should be preferred in case of duplicate keys.
        // This is considered implementation detail as per RFC-8259:
        //    "An object whose names are all unique is interoperable in the sense that all software implementations
        //    receiving that object will agree on the name-value mappings. When the names within an object are not
        //    unique, the behavior of software that receives such an object is unpredictable. Many implementations
        //    report the last name/value pair only. Other implementations report an error or fail to parse the object,
        //    and some implementations report all of the name/value pairs, including duplicates."

        // Note 3:
        // We could easily check for duplicate keys since 'std::map<>::emplace()' returns insertion success as a bool
        // (the same isn't true for 'std::map<>::emplace_hint()' which returns just the iterator), however we will
        // not since that goes against the standard

        // Note 4:
        // 'parent.emplace_hint(parent.end(), ...)' can drastically speed up parsing of sorted JSON objects, however
        // since most JSONs in the wild aren't sorted we will resort to a more generic option of regular '.emplace()'

        parent.try_emplace(std::move(key), std::move(value));

        return cursor;
    }

    std::pair<std::size_t, Object> parse_object(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening brace '{'

        // Empty object that will accumulate child nodes as we parse them
        Object object_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != '}') {
            cursor = this->parse_object_pair(cursor, object_value);
        } else {
            ++cursor; // move past the closing brace '}'
            return {cursor, std::move(object_value)};
        }

        // Handle other pairs

        // Since we are staring past the first pair, all following pairs are gonna be preceded by a comma.
        //
        // Strictly speaking, commas in objects aren't necessary for decoding a JSON, this is
        // a case of redundant information, included into the format to make it more human-readable.
        // { "key_1":1 "key_1":2 "key_3":"value" } <- enough information to parse without commas.
        //
        // However commas ARE in fact necessary for array parsing. By using commas to detect when we have
        // to parse another pair, we can reuse the same algorithm for both objects pairs and array elements.
        //
        // Doing so also has a benefit of inherently adding comma-presense validation which we would have
        // to do manually otherwise.

        while (cursor < this->chars.size()) {
            cursor       = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_object_pair(cursor, object_value);
            } else if (c == '}') {
                ++cursor; // move past the closing brace '}'
                return {cursor, std::move(object_value)};
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or object ending symbol {}} after the element at pos "s +
                    std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON object node reached the end of buffer while parsing object contents." +
                                 pretty_error(cursor, this->chars));
    }

    std::size_t parse_array_element(std::size_t cursor, Array& parent) {
        using namespace std::string_literals;

        // Array element parser assumes it is starting at the first symbol of some JSON node

        // Parse pair key
        if (++this->recursion_depth > this->recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                                     std::to_string(this->recursion_limit) +
                                     ". If stated depth wasn't caused by an invalid input, "s +
                                     "recursion limit can be increased with json::set_recursion_limit()."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        parent.emplace_back(std::move(value));

        return cursor;
    }

    std::pair<std::size_t, Array> parse_array(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening bracket '['

        // Empty array that will accumulate child nodes as we parse them
        Array array_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ']') {
            cursor = this->parse_array_element(cursor, array_value);
        } else {
            ++cursor; // move past the closing bracket ']'
            return {cursor, std::move(array_value)};
        }

        // Handle other pairs
        // (the exact same way we do with objects, see the note here)
        while (cursor < this->chars.size()) {
            cursor       = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_array_element(cursor, array_value);
            } else if (c == ']') {
                ++cursor; // move past the closing bracket ']'
                return {cursor, std::move(array_value)};
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or array ending symbol {]} after the element at pos "s +
                    std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON array node reached the end of buffer while parsing object contents." +
                                 pretty_error(cursor, this->chars));
    }

    inline std::size_t parse_escaped_unicode_codepoint(std::size_t cursor, std::string& string_value) {
        using namespace std::string_literals;

        // Note 1:
        // 4 hex digits can encode every character in a basic multilingual plane, to properly encode all valid unicode
        // chars 6 digits are needed. If JSON was a bit better we would have a longer escape sequence like '\Uxxxxxx',
        // (the way ECMAScript, Python and C++ do it), but for historical reasons longer codepoints are instead
        // represented using a UTF-16 surrogate pair like this: '\uxxxx\uxxxx'. The way such pair can be distinguished
        // from 2 independent codepoints is by checking a range of the first codepoint: values from 'U+D800' to 'U+DFFF'
        // are reserved for surrogate pairs. This is abhorrent and makes implementation twice as cumbersome, but we
        // gotta to do it in order to be standard-compliant.

        // Note 2:
        // 1st surrogate contains high bits, 2nd surrogate contains low bits.

        const auto throw_parsing_error = [&](std::string_view hex) {
            throw std::runtime_error("JSON string node could not parse unicode codepoint {"s + std::string(hex) +
                                     "} while parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));
        };

        const auto throw_surrogate_error = [&](std::string_view hex) {
            throw std::runtime_error("JSON string node encountered invalid unicode escape sequence in " +
                                     "second half of UTF-16 surrogate pair starting at {"s + std::string(hex) +
                                     "} while parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));
        };

        const auto throw_end_of_buffer_error = [&]() {
            throw std::runtime_error("JSON string node reached the end of buffer while "s +
                                     "parsing a unicode escape sequence at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));
        };

        const auto throw_end_of_buffer_error_for_pair = [&]() {
            throw std::runtime_error("JSON string node reached the end of buffer while "s +
                                     "parsing a unicode escape sequence surrogate pair at pos "s +
                                     std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
        };

        const auto parse_utf16 = [&](std::string_view hex) -> std::uint16_t {
            std::uint16_t utf16{};
            const auto [end_ptr, error_code] = std::from_chars(hex.data(), hex.data() + hex.size(), utf16, 16);

            const bool sequence_is_valid     = (error_code == std::errc{});
            const bool sequence_parsed_fully = (end_ptr == hex.data() + hex.size());

            if (!sequence_is_valid || !sequence_parsed_fully) throw_parsing_error(hex);

            return utf16;
        };

        // | '\uxxxx\uxxxx' | '\uxxxx\uxxxx'   | '\uxxxx\uxxxx' | '\uxxxx\uxxxx'   | '\uxxxx\uxxxx'  |
        // |   ^            |    ^             |       ^        |          ^       |             ^   |
        // | start (+0)     | hex_1_start (+1) | hex_1_end (+4) | hex_2_start (+7) | hex_2_end (+10) |
        constexpr std::size_t hex_1_start     = 1;
        constexpr std::size_t hex_1_end       = 4;
        constexpr std::size_t hex_2_backslash = 5;
        constexpr std::size_t hex_2_prefix    = 6;
        constexpr std::size_t hex_2_start     = 7;
        constexpr std::size_t hex_2_end       = 10;

        const auto start = this->chars.data() + cursor;

        if (cursor + hex_1_end >= this->chars.size()) throw_end_of_buffer_error();

        const std::string_view hex_1(start + hex_1_start, 4);
        const std::uint16_t    utf16_1 = parse_utf16(hex_1);

        // Surrogate pair case
        if (0xD800 <= utf16_1 && utf16_1 <= 0xDFFF) {
            if (cursor + hex_2_end >= this->chars.size()) throw_end_of_buffer_error_for_pair();
            if (start[hex_2_backslash] != '\\') throw_surrogate_error(hex_1);
            if (start[hex_2_prefix] != 'u') throw_surrogate_error(hex_1);

            const std::string_view hex_2(start + hex_2_start, 4);
            const std::uint16_t    utf16_2 = parse_utf16(hex_2);

            const std::uint32_t codepoint = utf16_pair_to_codepoint(utf16_1, utf16_2);
            if (!codepoint_to_utf8(string_value, codepoint)) throw_parsing_error(hex_1);
            return cursor + hex_2_end;
        }
        // Regular case
        else {
            const std::uint32_t codepoint = static_cast<std::uint32_t>(utf16_1);
            if (!codepoint_to_utf8(string_value, codepoint)) throw_parsing_error(hex_1);
            return cursor + hex_1_end;
        }
    }

    std::pair<std::size_t, String> parse_string(std::size_t cursor) {
        using namespace std::string_literals;

        // Empty string that will accumulate characters as we parse them
        std::string string_value;

        ++cursor; // move past the opening quote '"'

        // Serialize string while handling escape sequences.
        //
        // Doing 'string_value += c' for every char is ~50-60% slower than appending whole string at once,
        // which is why we 'buffer' appends by keeping track of 'segment_start' and 'cursor', and appending
        // whole chunks of the buffer to 'string_value' when we encounter an escape sequence or end of the string.
        //
        for (std::size_t segment_start = cursor; cursor < this->chars.size(); ++cursor) {
            const char c = this->chars[cursor];

            // Reached the end of the string
            if (c == '"') {
                string_value.append(this->chars.data() + segment_start, cursor - segment_start);
                ++cursor; // move past the closing quote '"'
                return {cursor, std::move(string_value)};
            }
            // Handle escape sequences inside the string
            else if (c == '\\') {
                ++cursor; // move past the backslash '\'

                string_value.append(this->chars.data() + segment_start, cursor - segment_start - 1);
                // can't buffer more than that since we have to insert special characters now

                if (cursor >= this->chars.size())
                    throw std::runtime_error("JSON string node reached the end of buffer while"s +
                                             "parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                                             pretty_error(cursor, this->chars));

                const char escaped_char = this->chars[cursor];

                // 2-character escape sequences
                if (const char replacement_char = lookup_parsed_escaped_chars[u8(escaped_char)]) {
                    string_value += replacement_char;
                }
                // 6/12-character escape sequences (escaped unicode HEX codepoints)
                else if (escaped_char == 'u') {
                    cursor = this->parse_escaped_unicode_codepoint(cursor, string_value);
                    // moves past first 'uXXX' symbols, last symbol will be covered by the loop '++cursor',
                    // in case of paired hexes moves past the second hex too
                } else {
                    throw std::runtime_error("JSON string node encountered unexpected character {"s +
                                             std::string{escaped_char} + "} while parsing an escape sequence at pos "s +
                                             std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
                }

                // This covers all non-hex escape sequences according to ECMA-404 specification
                // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf] (page 4)

                // moving past the escaped character will be done by the loop '++cursor'
                segment_start = cursor + 1;
                continue;
            }
            // Reject unescaped control characters (codepoints U+0000 to U+001F)
            else if (u8(c) <= 31)
                throw std::runtime_error(
                    "JSON string node encountered unescaped ASCII control character character \\"s +
                    std::to_string(static_cast<int>(c)) + " at pos "s + std::to_string(cursor) + "."s +
                    pretty_error(cursor, this->chars));
        }

        throw std::runtime_error("JSON string node reached the end of buffer while parsing string contents." +
                                 pretty_error(cursor, this->chars));
    }

    std::pair<std::size_t, Number> parse_number(std::size_t cursor) {
        using namespace std::string_literals;

        Number number_value;

        const auto [numer_end_ptr, error_code] = available_from_chars_impl(
            this->chars.data() + cursor, this->chars.data() + this->chars.size(), number_value);

        // Note:
        // std::from_chars() converts the first complete number it finds in the string,
        // for example "42 meters" would be converted to 42. We rely on that behaviour here.

        if (error_code != std::errc{}) {
            // std::errc(0) is a valid enumeration value that represents success
            // even though it does not appear in the enumerator list (which starts at 1)
            if (error_code == std::errc::invalid_argument)
                throw std::runtime_error("JSON number node could not be parsed as a number at pos "s +
                                         std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
            else if (error_code == std::errc::result_out_of_range)
                throw std::runtime_error(
                    "JSON number node parsed to number larger than its possible binary representation at pos "s +
                    std::to_string(cursor) + "."s + pretty_error(cursor, this->chars));
        }

        return {numer_end_ptr - this->chars.data(), number_value};
    }

    std::pair<std::size_t, Bool> parse_true(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {true}." +
                                     pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 't' && //
            this->chars[cursor + 1] == 'r' && //
            this->chars[cursor + 2] == 'u' && //
            this->chars[cursor + 3] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {true} at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));

        return {cursor + token_length, Bool(true)};
    }

    std::pair<std::size_t, Bool> parse_false(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 5;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {false}." +
                                     pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'f' && //
            this->chars[cursor + 1] == 'a' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 's' && //
            this->chars[cursor + 4] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {false} at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));

        return {cursor + token_length, Bool(false)};
    }

    std::pair<std::size_t, Null> parse_null(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON null node reached the end of buffer while parsing {null}." +
                                     pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'n' && //
            this->chars[cursor + 1] == 'u' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 'l';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON null node could not parse {null} at pos "s + std::to_string(cursor) + "."s +
                                     pretty_error(cursor, this->chars));

        return {cursor + token_length, Null()};
    }
};


// ==============================
// --- JSON Serializing impl. ---
// ==============================

template <bool prettify>
inline void serialize_json_recursion(const Node& node, std::string& chars, unsigned int indent_level = 0,
                                     bool skip_first_indent = false) {
    using namespace std::string_literals;
    constexpr std::size_t indent_level_size = 4;
    const std::size_t     indent_size       = indent_level_size * indent_level;

    // First indent should be skipped when printing after a key, for example:
    //    > {
    //    >     "object": {              <- first indent skipped (Object)
    //    >         "something": null    <- first indent skipped (Null)
    //    >     },
    //    >     "array": [               <- first indent skipped (Array)
    //    >          1,                  <- first indent NOT skipped (Number)
    //    >          2                   <- first indent NOT skipped (Number)
    //    >     ]
    //    > }

    // We handle 'prettify' segments through 'if constexpr'
    // to avoid any additional overhead on non-prettified serializing

    // Note:
    // The fastest way to append strings to a preallocated buffer seems to be with '+=':
    //    > chars += string_1; chars += string_2; chars += string_3;
    //
    // Using operator '+' slows things down due to additional allocations:
    //    > chars +=  string_1 + string_2 + string_3; // slow
    //
    // '.append()' performs exactly the same as '+=', but has no overload for appending single chars.
    // However, it does have an overload for appending N of some character, which is why we use if for indentation.
    //
    // 'std::ostringstream' is painfully slow compared to regular appends so it's out of the question.

    if constexpr (prettify)
        if (!skip_first_indent) chars.append(indent_size, ' ');

    // JSON Object
    if (auto* ptr = node.get_if<Object>()) {
        const auto& object_value = *ptr;

        // Skip all logic for empty objects
        if (object_value.empty()) {
            chars += "{}";
            return;
        }

        chars += '{';
        if constexpr (prettify) chars += '\n';

        for (auto it = object_value.cbegin();;) {
            if constexpr (prettify) chars.append(indent_size + indent_level_size, ' ');
            // Key
            chars += '"';
            chars += it->first;
            if constexpr (prettify) chars += "\": ";
            else chars += "\":";
            // Value
            serialize_json_recursion<prettify>(it->second, chars, indent_level + 1, true);
            // Comma
            if (++it != object_value.cend()) { // prevents trailing comma
                chars += ',';
                if constexpr (prettify) chars += '\n';
            } else {
                if constexpr (prettify) chars += '\n';
                break;
            }
        }

        if constexpr (prettify) chars.append(indent_size, ' ');
        chars += '}';
    }
    // JSON Array
    else if (auto* ptr = node.get_if<Array>()) {
        const auto& array_value = *ptr;

        // Skip all logic for empty arrays
        if (array_value.empty()) {
            chars += "[]";
            return;
        }

        chars += '[';
        if constexpr (prettify) chars += '\n';

        for (auto it = array_value.cbegin();;) {
            // Node
            serialize_json_recursion<prettify>(*it, chars, indent_level + 1);
            // Comma
            if (++it != array_value.cend()) { // prevents trailing comma
                chars += ',';
                if constexpr (prettify) chars += '\n';
            } else {
                if constexpr (prettify) chars += '\n';
                break;
            }
        }
        if constexpr (prettify) chars.append(indent_size, ' ');
        chars += ']';
    }
    // String
    else if (auto* ptr = node.get_if<String>()) {
        const auto& string_value = *ptr;

        chars += '"';

        // Serialize string while handling escape sequences.
        /// Without escape sequences we could just do 'chars += string_value'.
        //
        // Since appending individual characters is ~twice as slow as appending the whole string, we use a
        // "buffered" way of appending, appending whole segments up to the currently escaped char.
        // Strings with no escaped chars get appended in a single call.
        //
        std::size_t segment_start = 0;
        for (std::size_t i = 0; i < string_value.size(); ++i) {
            if (const char escaped_char_replacement = lookup_serialized_escaped_chars[u8(string_value[i])]) {
                chars.append(string_value.data() + segment_start, i - segment_start);
                chars += '\\';
                chars += escaped_char_replacement;
                segment_start = i + 1; // skip over the "actual" technical character in the string
            }
        }
        chars.append(string_value.data() + segment_start, string_value.size() - segment_start);

        chars += '"';
    }
    // Number
    else if (auto* ptr = node.get_if<Number>()) {
        const auto& number_value = *ptr;

        constexpr int max_exponent = std::numeric_limits<Number>::max_exponent10;
        constexpr int max_digits =
            4 + std::numeric_limits<Number>::max_digits10 + std::max(2, log10_ceil(max_exponent));
        // should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
        // see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

        std::array<char, max_digits> buffer;

        const auto [number_end_ptr, error_code] =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), number_value);

        if (error_code != std::errc{})
            throw std::runtime_error(
                "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                std::to_string(number_value) + "}."s);

        const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

        // Save NaN/Inf cases as strings, since JSON spec doesn't include IEEE 754.
        // (!) May result in non-homogenous arrays like [ 1.0, "inf" , 3.0, 4.0, "nan" ]
        if (std::isfinite(number_value)) {
            chars.append(buffer.data(), number_end_ptr - buffer.data());
        } else {
            chars += '"';
            chars.append(buffer.data(), number_end_ptr - buffer.data());
            chars += '"';
        }
    }
    // Bool
    else if (auto* ptr = node.get_if<Bool>()) {
        const auto& bool_value = *ptr;
        chars += (bool_value ? "true" : "false");
    }
    // Null
    else if (node.is<Null>()) {
        chars += "null";
    }
}

inline void serialize_json_to_buffer(std::string& chars, const Node& node, Format format) {
    if (format == Format::PRETTY) serialize_json_recursion<true>(node, chars);
    else serialize_json_recursion<false>(node, chars);
}

// ========================
// --- JSON parsing API ---
// ========================

[[nodiscard]] inline Node from_string(const std::string& chars,
                                      unsigned int       recursion_limit = default_recursion_limit) {
    Parser            parser(chars, recursion_limit);
    const std::size_t json_start = parser.skip_nonsignificant_whitespace(0); // skip leading whitespace
    auto [end_cursor, node]      = parser.parse_node(json_start); // starts parsing recursively from the root node

    // Check for invalid trailing symbols
    using namespace std::string_literals;

    for (auto cursor = end_cursor; cursor < chars.size(); ++cursor)
        if (!lookup_whitespace_chars[u8(chars[cursor])])
            throw std::runtime_error("Invalid trailing symbols encountered after the root JSON node at pos "s +
                                     std::to_string(cursor) + "."s + pretty_error(cursor, chars));

    return std::move(node); // implicit tuple blocks copy elision, we have to move() manually

    // Note: Some code analyzers detect 'return std::move(node)' as a performance issue, it is
    //       not, NOT having 'std::move()' on the other hand is very much a performance issue
}
[[nodiscard]] inline Node from_file(const std::string& filepath,
                                    unsigned int       recursion_limit = default_recursion_limit) {
    const std::string chars = read_file_to_string(filepath);
    return from_string(chars, recursion_limit);
}

namespace literals {
[[nodiscard]] inline Node operator""_utl_json(const char* c_str, std::size_t c_str_size) {
    return from_string(std::string(c_str, c_str_size));
}
} // namespace literals

// ============================
// --- Structure reflection ---
// ============================

// --- from-struct utils ---
// -------------------------

template <class T>
constexpr bool is_reflected_struct = false;
// this trait allows us to "mark" all reflected struct types, we use it to handle nested classes
// and call 'to_struct()' / 'from_struct()' recursively whenever necessary

template <class T>
[[nodiscard]] utl::json::impl::Node from_struct(const T&) {
    static_assert(always_false_v<T>,
                  "Provided type doesn't have a defined JSON reflection. Use 'UTL_JSON_REFLECT' macro to define one.");
    // compile-time protection against calling 'from_struct()' on types that don't have reflection,
    // we can also provide a proper error message here
    return {};
    // this is needed to silence "no return in a function" warning that appears even if this specialization
    // (which by itself should cause a compile error) doesn't get compiled
}

template <class T>
void assign_value_to_node(Node& node, const T& value) {
    if constexpr (is_json_convertible_v<T>) node = value;
    // it is critical that the trait above performs DEEP check for JSON convertibility and not a shallow one,
    // we want to detect things like 'std::vector<int>' as convertible, but not things like 'std::vector<MyStruct>',
    // these should expand over their element type / mapped type further until either they either reach
    // the reflected 'MyStruct' or end up on a dead end, which means an impossible conversion
    else if constexpr (is_reflected_struct<T>) node = from_struct(value);
    else if constexpr (is_object_like_v<T>) {
        node = Object{};
        for (const auto& [key, val] : value) {
            Node single_node;
            assign_value_to_node(single_node, val);
            node.get_object().emplace(key, std::move(single_node));
        }
    } else if constexpr (is_array_like_v<T>) {
        node = Array{};
        for (const auto& elem : value) {
            Node single_node;
            assign_value_to_node(single_node, elem);
            node.get_array().emplace_back(std::move(single_node));
        }
    } else static_assert(always_false_v<T>, "Could not resolve recursive conversion from 'T' to 'json::Node'.");
}

#define utl_json_from_struct_assign(fieldname_) assign_value_to_node(json[#fieldname_], val.fieldname_);

// --- to-struct utils ---
// -----------------------

// Assigning JSON node to a value for arbitrary type is a bit of an "incorrect" problem,
// since we can't possibly know the API of the type we're assigning stuff to.
// Object-like and array-like types need special handling that expands their nodes recursively,
// we can't directly assign 'std::vector<Node>' to 'std::vector<double>' like we would with simpler types.
template <class T>
void assign_node_to_value_recursively(T& value, const Node& node) {
    if constexpr (is_string_like_v<T>) value = node.get_string();
    else if constexpr (is_object_like_v<T>) {
        const auto object = node.get_object();
        for (const auto& [key, val] : object) assign_node_to_value_recursively(value[key], val);
    } else if constexpr (is_array_like_v<T>) {
        const auto array = node.get_array();
        value.resize(array.size());
        for (std::size_t i = 0; i < array.size(); ++i) assign_node_to_value_recursively(value[i], array[i]);
    } else if constexpr (is_bool_like_v<T>) value = node.get_bool();
    else if constexpr (is_null_like_v<T>) value = node.get_null();
    else if constexpr (is_numeric_like_v<T>) value = static_cast<T>(node.get_number());
    else if constexpr (is_reflected_struct<T>) value = node.to_struct<T>();
    else static_assert(always_false_v<T>, "Method is a non-exhaustive visitor of std::variant<>.");
}

// Not sure how to generically handle array-like types with compile-time known size,
// so we're just going to make a special case for 'std::array'
template <class T, std::size_t N>
void assign_node_to_value_recursively(std::array<T, N>& value, const Node& node) {
    using namespace std::string_literals;

    const auto array = node.get_array();

    if (array.size() != value.size())
        throw std::runtime_error("JSON to structure serializer encountered non-mathing std::array size of "s +
                                 std::to_string(value.size()) + ", corresponding node has a size of "s +
                                 std::to_string(array.size()) + "."s);

    for (std::size_t i = 0; i < array.size(); ++i) assign_node_to_value_recursively(value[i], array[i]);
}

#define utl_json_to_struct_assign(fieldname_)                                                                          \
    if (this->contains(#fieldname_)) assign_node_to_value_recursively(val.fieldname_, this->at(#fieldname_));
// JSON might not have an entry corresponding to each structure member,
// such members will stay defaulted according to the struct constructor

// --- Codegen ---
// ---------------

#define UTL_JSON_REFLECT(struct_name_, ...)                                                                            \
                                                                                                                       \
    template <>                                                                                                        \
    constexpr bool utl::json::impl::is_reflected_struct<struct_name_> = true;                                          \
                                                                                                                       \
    template <>                                                                                                        \
    inline utl::json::impl::Node utl::json::impl::from_struct<struct_name_>(const struct_name_& val) {                 \
        utl::json::impl::Node json;                                                                                    \
        /* map 'json["<FIELDNAME>"] = val.<FIELDNAME>;' */                                                             \
        utl_json_map(utl_json_from_struct_assign, __VA_ARGS__);                                                        \
        return json;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template <>                                                                                                        \
    inline auto utl::json::impl::Node::to_struct<struct_name_>() const->struct_name_ {                                 \
        struct_name_ val;                                                                                              \
        /* map 'val.<FIELDNAME> = this->at("<FIELDNAME>").get<decltype(val.<FIELDNAME>)>();' */                        \
        utl_json_map(utl_json_to_struct_assign, __VA_ARGS__);                                                          \
        return val;                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    static_assert(true)


} // namespace utl::json::impl

// ______________________ PUBLIC API ______________________

namespace utl::json {

using impl::Format;

using impl::Node;

using impl::Object;
using impl::Array;
using impl::String;
using impl::Number;
using impl::Bool;
using impl::Null;

using impl::from_string;
using impl::from_file;
using impl::from_struct;

namespace literals = impl::literals;

// macro -> UTL_JSON_REFLECT

using impl::is_reflected_struct;

} // namespace utl::json

#endif
#endif // module utl::json





// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::log
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_log.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_LOG)
#ifndef UTLHEADERGUARD_LOG
#define UTLHEADERGUARD_LOG

#define UTL_LOG_VERSION_MAJOR 0 // [!] module awaiting a rewrite
#define UTL_LOG_VERSION_MINOR 1
#define UTL_LOG_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>         // array<>
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock
#include <cstddef>       // size_t
#include <exception>     // exception
#include <fstream>       // ofstream
#include <iostream>      // cout
#include <iterator>      // next()
#include <limits>        // numeric_limits<>
#include <list>          // list<>
#include <mutex>         // lock_guard<>, mutex
#include <ostream>       // ostream
#include <sstream>       // std::ostringstream
#include <stdexcept>     // std::runtime_error
#include <string>        // string
#include <string_view>   // string_view
#include <system_error>  // errc()
#include <thread>        // this_thread::get_id()
#include <tuple>         // tuple_size<>
#include <type_traits>   // is_integral_v<>, is_floating_point_v<>, is_same_v<>, is_convertible_to_v<>
#include <unordered_map> // unordered_map<>
#include <utility>       // forward<>()
#include <variant>       // variant<>

// ____________________ DEVELOPER DOCS ____________________

// Reasonably performant and convenient logger.
//
// The main highlight of this module (and the main performance win relative to 'std::ostream') is a
// generic stringifier class that is both convenient and quite fast at formatting multiple values.
//
// This stringifier can be customized with CRTP to format things in all kinds of specific ways
// and will likely come in handy in some other modules.
//
// The logger implementation itself is actually not that efficient (even though not completely
// naive either), a proper performance-oriented approach would use following things:
//
//    1. More 'constexpr' things to avoid having to constantly check styling conditions at runtime
//
//    2. A separate persistent thread to flush the buffer
//
//       Note: I did try using a stripped down version of 'utl::parallel::ThreadPool' to upload tasks
//             for flushing the buffer, it generally improves performance by ~30%, however I decided it
//             is not worth the added complexity & cpu usage for that little gain
//
//    3. Platform-specific methods to query stuff like time & thread id with less overhead
//
//    4. A centralized formatting & info querying facility so multiple sinks don't have to repeat
//       formatting & querying logic.
//
//       Such facility would have to decide the bare minimum of work possible base on all existing
//       sink options, perform the formatting, and then just distribute string view to actual sinks.
//
//       This drastically complicates the logic and can be rather at odds with point (1) since unlike
//       the individual sinks, I don't see a way of making style checks here constexpr, but in the end
//       that would be a proper solution.

// ____________________ IMPLEMENTATION ____________________

namespace utl::log::impl {

// ======================
// --- Internal utils ---
// ======================

// - SFINAE to select localtime_s() or localtime_r() -
template <class TimeMoment, class TimeType>
auto available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer))) {
    return localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer));
}

template <class TimeMoment, class TimeType>
auto available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment))) {
    return localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment));
}

inline std::size_t get_thread_index(const std::thread::id id) {
    static std::mutex     mutex;
    const std::lock_guard lock(mutex);

    static std::unordered_map<std::thread::id, std::size_t> thread_ids;
    static std::size_t                                      next_id = 0;

    const auto it = thread_ids.find(id);
    if (it == thread_ids.end()) return thread_ids[id] = next_id++;
    return it->second;
    // this map effectively "demangles" platform-specific IDs into human-readable IDs (0, 1, 2, ...)
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
unsigned int integer_digit_count(IntType value) {
    unsigned int digits = (value <= 0) ? 1 : 0;
    // (value <  0) => we add 1 digit because of '-' in front
    // (value == 0) => we add 1 digit for '0' because the loop doesn't account for zero integers
    while (value) {
        value /= 10;
        ++digits;
    }
    return digits;
    // Note: There is probably a faster way of doing it
}

using clock = std::chrono::steady_clock;

inline const clock::time_point program_entry_time_point = clock::now();

// ===================
// --- Stringifier ---
// ===================

// --- Internal type traits ---
// ----------------------------

#define utl_log_define_trait(trait_name_, ...)                                                                         \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

utl_log_define_trait(has_real, std::declval<T>().real());
utl_log_define_trait(has_imag, std::declval<T>().imag());
utl_log_define_trait(has_string, std::declval<T>().string());
utl_log_define_trait(has_begin, std::declval<T>().begin());
utl_log_define_trait(has_end, std::declval<T>().end());
utl_log_define_trait(has_input_it, std::next(std::declval<T>().begin()));
utl_log_define_trait(has_get, std::get<0>(std::declval<T>()));
utl_log_define_trait(has_tuple_size, std::tuple_size<T>::value);
utl_log_define_trait(has_container_type, std::declval<typename std::decay_t<T>::container_type>());
utl_log_define_trait(has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());
utl_log_define_trait(is_pad_left, std::declval<std::decay_t<T>>().is_pad_left);
utl_log_define_trait(is_pad_right, std::declval<std::decay_t<T>>().is_pad_right);
utl_log_define_trait(is_pad, std::declval<std::decay_t<T>>().is_pad);

// Note:
// Trait '_has_input_it' is trickier than it may seem. Just doing '++std::declval<T>().begin()' will work
// most of the time, but there are cases where it returns 'false' for very much iterable types.
//
// """
//    Although the expression '++c.begin()' often compiles, it is not guaranteed to do so: 'c.begin()' is an rvalue
//    expression, and there is no LegacyInputIterator requirement that specifies that increment of an rvalue is
///   guaranteed to work. In particular, when iterators are implemented as pointers or its operator++ is
//    lvalue-ref-qualified, '++c.begin()' does not compile, while 'std::next(c.begin())' does.
// """ (c) https://en.cppreference.com/w/cpp/iterator/next
//
// By checking if 'std::next(c.begin())' compiles we can properly check that iterator satisfies input iterator
// requirements, which means we can use it with operator '++' to iterate over the container. Trying to just
// check for operator '++' would lead to false positives, while checking '++c.begin()' would lead to false
// negatives on containers such as 'std::array'. It would seem that 'std::iterator_traits' is the way to go,
// but since it provides no good way to test if iterator satisfies a category without checking every possible
// tag, it ends up being more verbose that existing solution.

#undef utl_log_define_trait

// --- Internal utils ---
// ----------------------

template <class>
constexpr bool always_false_v = false;

template <class T>
constexpr int log10_ceil(T num) {
    return num < 10 ? 1 : 1 + log10_ceil(num / 10);
}

template <class T>
constexpr int max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, log10_ceil(std::numeric_limits<T>::max_exponent10));
// should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
// see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

template <class T>
constexpr int max_int_digits = 2 + std::numeric_limits<T>::digits10;
// +2 because 'digits10' returns last digit index rather than the number of digits
// (aka 1 less than one would expect) and doesn't account for possible '-'.
// Also note that ints use 'digits10' and not 'max_digits10' which is only valid for floats.

// "Unwrapper" for container adaptors such as 'std::queue', 'std::priority_queue', 'std::stack'
template <class Adaptor>
const auto& underlying_container_cref(const Adaptor& adaptor) {

    struct Hack : private Adaptor {
        static const typename Adaptor::container_type& get_container(const Adaptor& adp) {
            return adp.*&Hack::c;
            // An extremely hacky yet standard-compliant way of accessing protected member
            // of a class without actually creating any instances of derived class.
            //
            // This is essentially 2 operators: '.*' and '&',
            // '.*' takes an object on the left side, and a member pointer on the right side,
            // and resolves the pointed-to member of the given object, this means
            // 'object.*&Class::member' is essentially equivalent to 'object.member',
            // except it reveals a "loophole" in protection semantics that allows us to interpret
            // base class object as if it was derived class.
            //
            // Note that doing seemingly much more logical 'static_cast<Derived&>(object).member'
            // is technically UB, even through most compilers will do the reasonable thing.
        }
    };

    return Hack::get_container(adaptor);
}

// --- Alignment ---
// -----------------

// To align/pad values in a stringifier we can wrap then in thin structs
// that gets some special alignment logic in stringifier formatting selector.

template <class T>
struct PadLeft {
    constexpr PadLeft(const T& val, std::size_t size) : val(val), size(size) {} // this is needed for CTAD
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_left = true;
}; // pads value the left (aka right alignment)

template <class T>
struct PadRight {
    constexpr PadRight(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_right = true;
}; // pads value the right (aka left alignment)

template <class T>
struct Pad {
    constexpr Pad(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad = true;
}; // pads value on both sides (aka center alignment)

constexpr std::string_view indent = "    ";

// --- Stringifier ---
// -------------------

// Generic stringifier with customizable API. Formatting of specific types can be customized by inheriting it
// and overriding specific methods. This is a reference implementation that is likely to be used in other modules.
//
// For example of how to extend stringifier see 'utl::log' documentation.
//
template <class Derived>
struct StringifierBase {
    using self    = StringifierBase;
    using derived = Derived;

    // --- Type-wise methods ---
    // -------------------------

    template <class T>
    static void append_bool(std::string& buffer, const T& value) {
        buffer += value ? "true" : "false";
    }

    template <class T>
    static void append_int(std::string& buffer, const T& value) {
        std::array<char, max_int_digits<T>> str;
        const auto [number_end_ptr, error_code] = std::to_chars(str.data(), str.data() + str.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing an integer.");
        buffer.append(str.data(), number_end_ptr - str.data());
    }

    template <class T>
    static void append_enum(std::string& buffer, const T& value) {
        derived::append_int(buffer, static_cast<std::underlying_type_t<T>>(value));
    }

    template <class T>
    static void append_float(std::string& buffer, const T& value) {
        std::array<char, max_float_digits<T>> str;
        const auto [number_end_ptr, error_code] = std::to_chars(str.data(), str.data() + str.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing a float.");
        buffer.append(str.data(), number_end_ptr - str.data());
    }

    template <class T>
    static void append_complex(std::string& buffer, const T& value) {
        derived::append_float(buffer, value.real());
        buffer += " + ";
        derived::append_float(buffer, value.imag());
        buffer += " i";
    }

    template <class T>
    static void append_path(std::string& buffer, const T& value) {
        buffer += value.string();
    }

    template <class T>
    static void append_string(std::string& buffer, const T& value) {
        buffer += value;
    }

    template <class T>
    static void append_array(std::string& buffer, const T& value) {
        buffer += "{ ";
        if (value.begin() != value.end())
            for (auto it = value.begin();;) {
                derived::append(buffer, *it);
                if (++it == value.end()) break; // prevents trailing comma
                buffer += ", ";
            }
        buffer += " }";
    }

    template <class T>
    static void append_tuple(std::string& buffer, const T& value) {
        self::append_tuple_fwd(buffer, value);
    }

    template <class T>
    static void append_adaptor(std::string& buffer, const T& value) {
        derived::append(buffer, underlying_container_cref(value));
    }

    template <class T>
    static void append_printable(std::string& buffer, const T& value) {
        buffer += (std::ostringstream() << value).str();
    }

    // --- Main API ---
    // ----------------

    template <class T>
    static void append(std::string& buffer, const T& value) {
        self::append_selector(buffer, value);
    }

    template <class... Args>
    static void append(std::string& buffer, const Args&... args) {
        (derived::append(buffer, args), ...);
    }

    template <class... Args>
    [[nodiscard]] static std::string stringify(Args&&... args) {
        std::string buffer;
        derived::append(buffer, std::forward<Args>(args)...);
        return buffer;
    }

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return derived::stringify(std::forward<Args>(args)...);
    } // allows stringifier to be used as a functor

    // --- Helpers ---
    // ---------------
private:
    template <class Tuple, std::size_t... Idx>
    static void append_tuple_impl(std::string& buffer, Tuple value, std::index_sequence<Idx...>) {
        ((Idx == 0 ? "" : buffer += ", ", derived::append(buffer, std::get<Idx>(value))), ...);
        // fold expression '( f(args), ... )' invokes 'f(args)' for all arguments in 'args...'
        // in the same fashion, we can fold over 2 functions by doing '( ( f(args), g(args) ), ... )'
    }

    template <template <class...> class Tuple, class... Args>
    static void append_tuple_fwd(std::string& buffer, const Tuple<Args...>& value) {
        buffer += "< ";
        self::append_tuple_impl(buffer, value, std::index_sequence_for<Args...>{});
        buffer += " >";
    }

    template <class T>
    static void append_selector(std::string& buffer, const T& value) {
        // Left-padded something
        if constexpr (is_pad_left_v<T>) {
            std::string temp;
            self::append_selector(temp, value.val);
            if (temp.size() < value.size) buffer.append(value.size - temp.size(), ' ');
            buffer += temp;
        }
        // Right-padded something
        else if constexpr (is_pad_right_v<T>) {
            const std::size_t old_size = buffer.size();
            self::append_selector(buffer, value.val);
            const std::size_t appended_size = buffer.size() - old_size;
            if (appended_size < value.size) buffer.append(value.size - appended_size, ' ');
            // right-padding is faster than left padding since we don't need a temporary string to get appended size
        }
        // Center-padded something
        else if constexpr (is_pad_v<T>) {
            std::string temp;
            self::append_selector(temp, value.val);
            if (temp.size() < value.size) {
                const std::size_t lpad_size = (value.size - temp.size()) / 2;
                const std::size_t rpad_size = value.size - lpad_size - temp.size();
                buffer.append(lpad_size, ' ');
                buffer += temp;
                buffer.append(rpad_size, ' ');
            } else buffer += temp;
        }
        // Bool
        else if constexpr (std::is_same_v<T, bool>)
            derived::append_bool(buffer, value);
        // Char
        else if constexpr (std::is_same_v<T, char>) derived::append_string(buffer, value);
        // 'std::filesystemp::path'-like
        else if constexpr (has_string_v<T>) derived::append_path(buffer, value);
        // 'std::string_view'-convertible (most strings and string-like types)
        else if constexpr (std::is_convertible_v<T, std::string_view>) derived::append_string(buffer, value);
        // 'std::string'-convertible (some "nastier" string-like types, mainly 'std::path')
        else if constexpr (std::is_convertible_v<T, std::string>) derived::append_string(buffer, std::string(value));
        // Integral
        else if constexpr (std::is_integral_v<T>) derived::append_int(buffer, value);
        // Enum
        else if constexpr (std::is_enum_v<T>) derived::append_enum(buffer, value);
        // Floating-point
        else if constexpr (std::is_floating_point_v<T>) derived::append_float(buffer, value);
        // Complex
        else if constexpr (has_real_v<T> && has_imag_v<T>) derived::append_complex(buffer, value);
        // Array-like
        else if constexpr (has_begin_v<T> && has_end_v<T> && has_input_it_v<T>) derived::append_array(buffer, value);
        // Tuple-like
        else if constexpr (has_get_v<T> && has_tuple_size_v<T>) derived::append_tuple(buffer, value);
        // Container adaptor
        else if constexpr (has_container_type_v<T>) derived::append_adaptor(buffer, value);
        // 'std::ostream' printable
        else if constexpr (has_ostream_insert_v<T>) derived::append_printable(buffer, value);
        // No valid stringification exists
        else static_assert(always_false_v<T>, "No valid stringification exists for the type.");

        // Note: Using if-constexpr chain here allows us to pick and choose priority of different branches,
        // removing any possible ambiguity we could encounter doing things through SFINAE or overloads.
    }
};

// Note:
// The stringifier shines at stringifying & concatenating multiple values into the same buffer.
// Single-value is a specific case which allows all 'buffer += ...' to be replaced with most things being formatted
// straight into a newly created string. We could optimize this, but that would make require an almost full logic
// duplication and make the class cumbersome to extend since instead of a singular 'append_something()' methods there
// would be 2: 'append_something()' and 'construct_something()'. It also doesn't seem to be worth it, the difference
// in performance isn't that significant and we're still faster than most usual approaches to stringification.

// ===============================
// --- Stringifier derivatives ---
// ===============================

// "Default" customization of stringifier, here we can optimize a few things.
//
// The reason we don't include this in the original stringifier is because it's intended to be a customizable
// class that can be extended/optimized/decorated by inheriting it and overriding specific methods. The changes
// made by some optimizations wouldn't be compatible with such philosophy.
//
struct Stringifier : public StringifierBase<Stringifier> {
    using base = StringifierBase<Stringifier>;

    // Optimization overloads
    using base::stringify;

    [[nodiscard]] static std::string stringify(int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long long arg) { return std::to_string(arg); }
    // for individual ints 'std::to_string()' beats 'append_int()' with <charconv> since any reasonable compiler
    // implements it using the same <charconv> routine, but formatted directly into the string upon its creation

    [[nodiscard]] static std::string stringify(std::string&& arg) { return std::move(arg); }
    // no need to do all the appending stuff for individual r-value strings, just forward them as is

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return Stringifier::stringify(std::forward<Args>(args)...);
    }
};

template <class... Args>
void append_stringified(std::string& str, Args&&... args) {
    Stringifier::append(str, std::forward<Args>(args)...);
}

template <class... Args>
[[nodiscard]] std::string stringify(Args&&... args) {
    return Stringifier::stringify(std::forward<Args>(args)...);
}

template <class... Args>
void print(Args&&... args) {
    const auto                        res = Stringifier::stringify(std::forward<Args>(args)...);
    static std::mutex                 mutex;
    const std::lock_guard<std::mutex> lock(mutex);
    std::cout << res << std::flush;
    // print in a thread-safe way and instantly flush every message, this is much slower that buffering
    // (which regular logging methods do), but for generic console output this is a more robust way
}

template <class... Args>
void println(Args&&... args) {
    print(std::forward<Args>(args)..., '\n');
}

// ===============
// --- Options ---
// ===============

enum class Verbosity { ERR = 0, WARN = 1, NOTE = 2, INFO = 3, DEBUG = 4, TRACE = 5 };

enum class OpenMode { REWRITE, APPEND };

enum class Colors { ENABLE, DISABLE };

struct Columns {
    bool datetime = true;
    bool uptime   = true;
    bool thread   = true;
    bool callsite = true;
    bool level    = true;
    bool message  = true;

    // Columns() : datetime(true), uptime(true), thread(true), callsite(true), level(true), message(true) {}
};

struct Callsite {
    std::string_view file;
    int              line;
};

struct MessageMetadata {
    Verbosity verbosity;
};

constexpr bool operator<(Verbosity l, Verbosity r) { return static_cast<int>(l) < static_cast<int>(r); }
constexpr bool operator<=(Verbosity l, Verbosity r) { return static_cast<int>(l) <= static_cast<int>(r); }

// --- Column widths ---
// ---------------------

constexpr unsigned int w_uptime_sec = 4;
constexpr unsigned int w_uptime_ms  = 3;

constexpr std::size_t w_callsite_before_dot = 22;
constexpr std::size_t w_callsite_after_dot  = 4;

constexpr std::size_t col_w_datetime = sizeof("yyyy-mm-dd HH:MM:SS") - 1;
constexpr std::size_t col_w_uptime   = w_uptime_sec + 1 + w_uptime_ms;
constexpr std::size_t col_w_thread   = sizeof("thread") - 1;
constexpr std::size_t col_w_callsite = w_callsite_before_dot + 1 + w_callsite_after_dot;
constexpr std::size_t col_w_level    = sizeof("level") - 1;

// --- Column left/right delimiters ---
// ------------------------------------

constexpr std::string_view col_ld_datetime = "";
constexpr std::string_view col_rd_datetime = " ";
constexpr std::string_view col_ld_uptime   = "(";
constexpr std::string_view col_rd_uptime   = ")";
constexpr std::string_view col_ld_thread   = "[";
constexpr std::string_view col_rd_thread   = "]";
constexpr std::string_view col_ld_callsite = " ";
constexpr std::string_view col_rd_callsite = " ";
constexpr std::string_view col_ld_level    = "";
constexpr std::string_view col_rd_level    = "|";
constexpr std::string_view col_ld_message  = " ";
constexpr std::string_view col_rd_message  = "\n";

// --- ANSI Colors ---
// -------------------

namespace color {

constexpr std::string_view black          = "\033[30m";
constexpr std::string_view red            = "\033[31m";
constexpr std::string_view green          = "\033[32m";
constexpr std::string_view yellow         = "\033[33m";
constexpr std::string_view blue           = "\033[34m";
constexpr std::string_view magenta        = "\033[35m";
constexpr std::string_view cyan           = "\033[36m";
constexpr std::string_view white          = "\033[37m";
constexpr std::string_view bright_black   = "\033[90m"; // also known as "gray"
constexpr std::string_view bright_red     = "\033[91m";
constexpr std::string_view bright_green   = "\033[92m";
constexpr std::string_view bright_yellow  = "\033[93m";
constexpr std::string_view bright_blue    = "\033[94m";
constexpr std::string_view bright_magenta = "\033[95m";
constexpr std::string_view bright_cyan    = "\033[96m";
constexpr std::string_view bright_white   = "\033[97m";

constexpr std::string_view bold_black          = "\033[30;1m";
constexpr std::string_view bold_red            = "\033[31;1m";
constexpr std::string_view bold_green          = "\033[32;1m";
constexpr std::string_view bold_yellow         = "\033[33;1m";
constexpr std::string_view bold_blue           = "\033[34;1m";
constexpr std::string_view bold_magenta        = "\033[35;1m";
constexpr std::string_view bold_cyan           = "\033[36;1m";
constexpr std::string_view bold_white          = "\033[37;1m";
constexpr std::string_view bold_bright_black   = "\033[90;1m";
constexpr std::string_view bold_bright_red     = "\033[91;1m";
constexpr std::string_view bold_bright_green   = "\033[92;1m";
constexpr std::string_view bold_bright_yellow  = "\033[93;1m";
constexpr std::string_view bold_bright_blue    = "\033[94;1m";
constexpr std::string_view bold_bright_magenta = "\033[95;1m";
constexpr std::string_view bold_bright_cyan    = "\033[96;1m";
constexpr std::string_view bold_bright_white   = "\033[97;1m";

constexpr std::string_view reset = "\033[0m";

// logger itself only uses a few of those colors, but since we do it this way might as well provide
// the whole spectrum so users can color 'cout' and 'log::println()' statements too

} // namespace color

constexpr std::string_view color_heading = color::bold_cyan;
constexpr std::string_view color_reset   = color::reset;

constexpr std::string_view color_trace = color::bright_black;
constexpr std::string_view color_debug = color::green;
constexpr std::string_view color_note  = color::magenta;
constexpr std::string_view color_info  = color::white;
constexpr std::string_view color_warn  = color::yellow;
constexpr std::string_view color_err   = color::bold_red;

// ==================
// --- Sink class ---
// ==================

class Sink {
private:
    using os_ref_wrapper = std::reference_wrapper<std::ostream>;

    std::variant<os_ref_wrapper, std::ofstream> os_variant;
    Verbosity                                   verbosity;
    Colors                                      colors;
    clock::duration                             flush_interval;
    Columns                                     columns;
    clock::time_point                           last_flushed;
    bool                                        print_header = true;
    mutable std::mutex                          ostream_mutex;

    friend struct Logger;

    std::ostream& ostream_ref() {
        if (const auto ref_wrapper_ptr = std::get_if<os_ref_wrapper>(&this->os_variant)) return ref_wrapper_ptr->get();
        else return std::get<std::ofstream>(this->os_variant);
    }

public:
    Sink()            = delete;
    Sink(const Sink&) = delete;
    Sink(Sink&&)      = delete;

    Sink(std::ofstream&& os, Verbosity verbosity, Colors colors, clock::duration flush_interval, const Columns& columns)
        : os_variant(std::move(os)), verbosity(verbosity), colors(colors), flush_interval(flush_interval),
          columns(columns) {}

    Sink(std::reference_wrapper<std::ostream> os, Verbosity verbosity, Colors colors, clock::duration flush_interval,
         const Columns& columns)
        : os_variant(os), verbosity(verbosity), colors(colors), flush_interval(flush_interval), columns(columns) {}

    // We want a way of changing sink options using its handle / reference returned by the logger
    Sink& set_verbosity(Verbosity verbosity) {
        this->verbosity = verbosity;
        return *this;
    }
    Sink& set_colors(Colors colors) {
        this->colors = colors;
        return *this;
    }
    Sink& set_flush_interval(clock::duration flush_interval) {
        this->flush_interval = flush_interval;
        return *this;
    }
    Sink& set_columns(const Columns& columns) {
        this->columns = columns;
        return *this;
    }
    Sink& skip_header(bool skip = true) {
        this->print_header = !skip;
        return *this;
    }

private:
    template <class... Args>
    void format(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        if (meta.verbosity > this->verbosity) return;

        thread_local std::string buffer;

        const clock::time_point now = clock::now();

        // To minimize logging overhead we use string buffer, append characters to it and then write the whole buffer
        // to `std::ostream`. This avoids the inherent overhead of ostream formatting (caused largely by
        // virtualization, syncronization and locale handling, neither of which are relevant for the logger).
        //
        // This buffer gets reused between calls. Note the 'thread_local', if buffer was just a class member, multiple
        // threads could fight trying to clear and resize the buffer while it's being written to by another thread.
        //
        // Buffer may grow when formatting a message longer that any one that was formatted before, otherwise we just
        // reuse the reserved memory and no new allocations take place.

        buffer.clear();

        // Print log header on the first call
        {
            static std::mutex     header_mutex;
            const std::lock_guard lock(header_mutex);
            if (this->print_header) {
                this->print_header = false;
                this->format_header(buffer);
            }
            // no need to lock the buffer, other threads can't reach buffer
            // output code while they're stuck waiting for the header to print
        }

        // Format columns one-by-one
        if (this->colors == Colors::ENABLE) switch (meta.verbosity) {
            case Verbosity::ERR: buffer += color_err; break;
            case Verbosity::WARN: buffer += color_warn; break;
            case Verbosity::NOTE: buffer += color_note; break;
            case Verbosity::INFO: buffer += color_info; break;
            case Verbosity::DEBUG: buffer += color_debug; break;
            case Verbosity::TRACE: buffer += color_trace; break;
            }

        if (this->columns.datetime) this->format_column_datetime(buffer);
        if (this->columns.uptime) this->format_column_uptime(buffer, now);
        if (this->columns.thread) this->format_column_thread(buffer);
        if (this->columns.callsite) this->format_column_callsite(buffer, callsite);
        if (this->columns.level) this->format_column_level(buffer, meta.verbosity);
        if (this->columns.message) this->format_column_message(buffer, args...);

        if (this->colors == Colors::ENABLE) buffer += color_reset;

        // 'std::ostream' isn't guaranteed to be thread-safe, even through many implementations seem to have
        // some thread-safety built into `std::cout` the same cannot be said about a generic 'std::ostream'
        const std::lock_guard ostream_lock(this->ostream_mutex);

        this->ostream_ref().write(buffer.data(), buffer.size());

        // flush every message immediately
        if (this->flush_interval.count() == 0) {
            this->ostream_ref().flush();
        }
        // or flush periodically
        else if (now - this->last_flushed > this->flush_interval) {
            this->last_flushed = now;
            this->ostream_ref().flush();
        }
    }

    void format_header(std::string& buffer) {
        if (this->colors == Colors::ENABLE) buffer += color_heading;
        if (this->columns.datetime)
            append_stringified(buffer, col_ld_datetime, PadRight{"date       time", col_w_datetime}, col_rd_datetime);
        if (this->columns.uptime)
            append_stringified(buffer, col_ld_uptime, PadRight{"uptime", col_w_uptime}, col_rd_uptime);
        if (this->columns.thread)
            append_stringified(buffer, col_ld_thread, PadRight{"thread", col_w_thread}, col_rd_thread);
        if (this->columns.callsite)
            append_stringified(buffer, col_ld_callsite, PadRight{"callsite", col_w_callsite}, col_rd_callsite);
        if (this->columns.level) append_stringified(buffer, col_ld_level, PadRight{"level", col_w_level}, col_rd_level);
        if (this->columns.message) append_stringified(buffer, col_ld_message, "message", col_rd_message);
        if (this->colors == Colors::ENABLE) buffer += color_reset;
    }

    void format_column_datetime(std::string& buffer) {
        std::time_t timer = std::time(nullptr);
        std::tm     time_moment{};

        available_localtime_impl(&time_moment, &timer);

        // Format time straight into the buffer
        std::array<char, col_w_datetime + 1> strftime_buffer; // size includes the null terminator added by 'strftime()'
        std::strftime(strftime_buffer.data(), strftime_buffer.size(), "%Y-%m-%d %H:%M:%S", &time_moment);

        // strftime_buffer.back() = ' '; // replace null-terminator added by 'strftime()' with a space
        buffer += col_ld_datetime;
        buffer.append(strftime_buffer.data(), col_w_datetime);
        buffer += col_rd_datetime;
    }

    void format_column_uptime(std::string& buffer, clock::time_point now) {
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - program_entry_time_point);
        const auto sec        = (elapsed_ms / 1000).count();
        const auto ms         = (elapsed_ms % 1000).count(); // is 'elapsed_ms - 1000 * full_seconds; faster?

        const unsigned int sec_digits = integer_digit_count(sec);
        const unsigned int ms_digits  = integer_digit_count(ms);

        buffer += col_ld_uptime;

        // Left-pad the value to column width (doing it manually is a bit faster than using PadLeft{})
        if (sec_digits < w_uptime_sec) buffer.append(w_uptime_sec - sec_digits, ' ');
        append_stringified(buffer, sec);

        buffer += '.';

        // Add leading zeroes to a fixed length
        if (ms_digits < w_uptime_ms) buffer.append(w_uptime_ms - ms_digits, '0');
        append_stringified(buffer, ms);

        buffer += col_rd_uptime;
    }

    void format_column_thread(std::string& buffer) {
        const auto thread_id       = get_thread_index(std::this_thread::get_id());
        const auto thread_id_width = integer_digit_count(thread_id);

        buffer += col_ld_thread;
        if (thread_id_width < col_w_thread) buffer.append(col_w_thread - thread_id_width, ' ');
        append_stringified(buffer, thread_id);
        buffer += col_rd_thread;
    }

    void format_column_callsite(std::string& buffer, const Callsite& callsite) {
        // Get just filename from the full path
        std::string_view filename = callsite.file.substr(callsite.file.find_last_of("/\\") + 1);

        // Left-pad callsite to column width, trim first characters if it's too long
        if (filename.size() < w_callsite_before_dot) buffer.append(w_callsite_before_dot - filename.size(), ' ');
        else filename.remove_prefix(w_callsite_before_dot - filename.size());

        buffer += col_ld_callsite;
        buffer += filename;
        buffer += ':';
        // Right-pad line number
        append_stringified(buffer, callsite.line);
        buffer.append(w_callsite_after_dot - integer_digit_count(callsite.line), ' ');
        buffer += col_rd_callsite;
    }

    void format_column_level(std::string& buffer, Verbosity level) {
        buffer += col_ld_level;
        switch (level) {
        case Verbosity::ERR: buffer += "  ERR"; break;
        case Verbosity::WARN: buffer += " WARN"; break;
        case Verbosity::NOTE: buffer += " NOTE"; break;
        case Verbosity::INFO: buffer += " INFO"; break;
        case Verbosity::DEBUG: buffer += "DEBUG"; break;
        case Verbosity::TRACE: buffer += "TRACE"; break;
        }
        buffer += col_rd_level;
    }

    template <class... Args>
    void format_column_message(std::string& buffer, const Args&... args) {
        buffer += col_ld_message;
        append_stringified(buffer, args...);
        buffer += col_rd_message;
    }
};

// ====================
// --- Logger class ---
// ====================

struct Logger {
    inline static std::list<Sink> sinks;
    // we use list<> because we don't want sinks to ever reallocate when growing / shrinking
    // (reallocation requires a move-constructor, which 'std::mutex' doesn't have),
    // the added overhead of iterating a list is negligible

    static inline Sink default_sink{std::cout, Verbosity::TRACE, Colors::ENABLE, std::chrono::milliseconds(0),
                                    Columns{}};

    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    template <class... Args>
    void push_message(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        // When no sinks were manually created, default sink-to-terminal takes over
        if (this->sinks.empty()) {
            // static Sink default_sink(std::cout, Verbosity::TRACE, Colors::ENABLE, ms(0), Columns{});
            default_sink.format(callsite, meta, args...);
        } else
            for (auto& sink : this->sinks) sink.format(callsite, meta, args...);
    }
};

// =======================
// --- Sink public API ---
// =======================

inline Sink& add_ostream_sink(std::ostream&   os,                                           //
                              Verbosity       verbosity      = Verbosity::INFO,             //
                              Colors          colors         = Colors::ENABLE,              //
                              clock::duration flush_interval = std::chrono::milliseconds{}, //
                              const Columns&  columns        = Columns{}                    //
) {
    return Logger::instance().sinks.emplace_back(os, verbosity, colors, flush_interval, columns);
}

inline Sink& add_file_sink(const std::string& filename,                                       //
                           OpenMode           open_mode      = OpenMode::REWRITE,             //
                           Verbosity          verbosity      = Verbosity::TRACE,              //
                           Colors             colors         = Colors::DISABLE,               //
                           clock::duration    flush_interval = std::chrono::milliseconds{15}, //
                           const Columns&     columns        = Columns{}                      //
) {
    const auto ios_open_mode = (open_mode == OpenMode::APPEND) ? std::ios::out | std::ios::app : std::ios::out;
    return Logger::instance().sinks.emplace_back(std::ofstream(filename, ios_open_mode), verbosity, colors,
                                                 flush_interval, columns);
}

// ======================
// --- Logging macros ---
// ======================

#define UTL_LOG_ERR(...)                                                                                               \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::ERR}, __VA_ARGS__)

#define UTL_LOG_WARN(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::WARN},           \
                                                    __VA_ARGS__)

#define UTL_LOG_NOTE(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::NOTE},           \
                                                    __VA_ARGS__)

#define UTL_LOG_INFO(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::INFO},           \
                                                    __VA_ARGS__)

#define UTL_LOG_DEBUG(...)                                                                                             \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::DEBUG},          \
                                                    __VA_ARGS__)

#define UTL_LOG_TRACE(...)                                                                                             \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::TRACE},          \
                                                    __VA_ARGS__)

#ifdef _DEBUG
#define UTL_LOG_DERR(...) UTL_LOG_ERR(__VA_ARGS__)
#define UTL_LOG_DWARN(...) UTL_LOG_WARN(__VA_ARGS__)
#define UTL_LOG_DNOTE(...) UTL_LOG_NOTE(__VA_ARGS__)
#define UTL_LOG_DINFO(...) UTL_LOG_INFO(__VA_ARGS__)
#define UTL_LOG_DDEBUG(...) UTL_LOG_DEBUG(__VA_ARGS__)
#define UTL_LOG_DTRACE(...) UTL_LOG_TRACE(__VA_ARGS__)
#else
#define UTL_LOG_DERR(...)
#define UTL_LOG_DWARN(...)
#define UTL_LOG_DNOTE(...)
#define UTL_LOG_DINFO(...)
#define UTL_LOG_DDEBUG(...)
#define UTL_LOG_DTRACE(...)
#endif

} // namespace utl::log::impl

// ______________________ PUBLIC API ______________________

namespace utl::log {

using impl::PadLeft;
using impl::PadRight;
using impl::Pad;

using impl::StringifierBase;
using impl::Stringifier;

using impl::append_stringified;
using impl::stringify;

using impl::print;
using impl::println;

using impl::Verbosity;
using impl::OpenMode;
using impl::Colors;

using impl::Columns;

using impl::Sink;

using impl::add_ostream_sink;
using impl::add_file_sink;

// macro -> ...

} // namespace utl::log

#endif
#endif // module utl::log






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::math
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_math.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MATH)
#ifndef UTLHEADERGUARD_MATH
#define UTLHEADERGUARD_MATH

#define UTL_MATH_VERSION_MAJOR 1
#define UTL_MATH_VERSION_MINOR 1
#define UTL_MATH_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <cassert>     // assert()
#include <limits>      // numeric_limits<>
#include <type_traits> // enable_if_t<>, is_floating_point<>, is_arithmetic<>, is_integral<>, is_same<>, ...

// ____________________ DEVELOPER DOCS ____________________

// A bunch of template utils that naturally accumulated over time.
//
// This header used to be somewhat of a junkyard of different math-adjacent stuff, but later underwent a cleanup
// due to the development of another numerical library (GSE) implementing a much more "serious" mathematical package,
// a lot of functionality from here was generalized and moved into the appropriate modules of this lib.

// ____________________ IMPLEMENTATION ____________________

namespace utl::math::impl {

// ======================
// --- SFINAE helpers ---
// ======================

template <bool Cond>
using require = std::enable_if_t<Cond, bool>;

template <class T>
using require_arithmetic = require<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>;

template <class T>
using require_int = require<std::is_integral_v<T> && !std::is_same_v<T, bool>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class T, class... Args>
using require_invocable = require<std::is_invocable_v<T, Args...>>;

// =================
// --- Constants ---
// =================

namespace constants {

constexpr double pi      = 3.14159265358979323846;
constexpr double two_pi  = 2.0 * pi;
constexpr double half_pi = 0.5 * pi;
constexpr double inv_pi  = 1.0 / pi;
constexpr double e       = 2.71828182845904523536;
constexpr double phi     = 1.61803398874989484820; // golden ratio

} // namespace constants

// =======================
// --- Basic functions ---
// =======================

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T abs(T x) noexcept {
    return (x > T(0)) ? x : -x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T sign(T x) noexcept {
    if constexpr (std::is_unsigned_v<T>) return (x > T(0)) ? T(1) : T(0);
    else return (x > T(0)) ? T(1) : (x < T(0)) ? T(-1) : T(0);
} // returns -1 / 0 / 1

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T bsign(T x) noexcept {
    if constexpr (std::is_unsigned_v<T>) return T(1);
    else return (x >= T(0)) ? T(1) : T(-1);
} // returns -1 / 1 (1 gets priority in x == 0)

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T sqr(T x) noexcept {
    return x * x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T cube(T x) noexcept {
    return x * x * x;
}

template <class T, require_float<T> = true>
[[nodiscard]] constexpr T inv(T x) noexcept {
    return T(1) / x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T heaviside(T x) noexcept {
    return static_cast<T>(x > T(0));
}

// Floating point midpoint base on 'libstdc++' implementation,
// takes care of extreme values
template <class T, require_float<T> = true>
[[nodiscard]] constexpr T midpoint(T a, T b) noexcept {
    constexpr T low  = std::numeric_limits<T>::min() * 2;
    constexpr T high = std::numeric_limits<T>::max() / 2;

    const T abs_a = abs(a);
    const T abs_b = abs(b);

    if (abs_a <= high && abs_b <= high) return (a + b) / 2; // always correctly rounded
    if (abs_a < low) return a + b / 2;                      // not safe to halve 'a'
    if (abs_b < low) return b + a / 2;                      // not safe to halve 'b'
    return a / 2 + b / 2;                                   // correctly rounded for remaining cases
}

// Non-overflowing integer midpoint is less trivial that it might initially seem, see
// https://lemire.me/blog/2022/12/06/fast-midpoint-between-two-integers-without-overflow/
// https://biowpn.github.io/bioweapon/2025/03/23/generalizing-std-midpoint.html
template <class T, require_int<T> = true>
[[nodiscard]] constexpr T midpoint(T a, T b) noexcept {
    return ((a ^ b) >> 1) + (a & b);
    // fast rounding-down midpoint by Warren (Hacker's Delight section 2.5)
    // rounding-up version would be '(a | b) - ((a ^ b) >> 1)'
    // this is faster than C++20 'std::midpoint()' due to a different rounding mode
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T absdiff(T a, T b) noexcept {
    return (a > b) ? (a - b) : (b - a);
}

// =======================
// --- Power functions ---
// =======================

// Squaring algorithm for positive integer powers
template <class T, class U, require_arithmetic<T> = true, require_int<U> = true>
[[nodiscard]] constexpr T pow_squaring(T x, U p) noexcept {
    if (p == U(0)) return T(1);
    if (p == U(1)) return x;
    const T half_pow = pow_squaring(x, p / 2);
    return (p % U(2) == U(0)) ? half_pow * half_pow : half_pow * half_pow * x;
}

template <class T, class U, require_arithmetic<T> = true, require_int<U> = true>
[[nodiscard]] constexpr T pow(T x, U p) noexcept {
    if constexpr (std::is_signed_v<T>) {
        return (p < 0) ? T(1) / pow_squaring(x, -p) : pow_squaring(x, p);
    } else {
        return pow_squaring(x, p); // no need for the branch in unsigned case
    }
}

[[nodiscard]] constexpr int signpow(int p) noexcept { return (p % 2 == 0) ? 1 : -1; }

// =======================
// --- Index functions ---
// =======================

template <class T, require_int<T> = true>
[[nodiscard]] constexpr T kronecker_delta(T i, T j) noexcept {
    return (i == j) ? T(1) : T(0);
}

template <class T, require_int<T> = true>
[[nodiscard]] constexpr T levi_civita(T i, T j, T k) noexcept {
    if (i == j || j == k || k == i) return T(0);
    const unsigned int inversions = (i > j) + (i > k) + (j > k);
    return (inversions % 2 == 0) ? T(1) : T(-1);
}

// ===================
// --- Conversions ---
// ===================

template <class T, require_float<T> = true>
[[nodiscard]] constexpr T deg_to_rad(T degrees) noexcept {
    constexpr T factor = T(constants::pi / 180.);
    return degrees * factor;
}

template <class T, require_float<T> = true>
[[nodiscard]] constexpr T rad_to_deg(T radians) noexcept {
    constexpr T factor = T(180. / constants::pi);
    return radians * factor;
}

// ===========================
// --- Sequence operations ---
// ===========================

template <class Idx, class Func, require_invocable<Func, Idx> = true>
[[nodiscard]] constexpr auto sum(Idx low, Idx high, Func&& func) noexcept(noexcept(func(Idx{}))) {
    assert(low <= high);
    std::invoke_result_t<Func, Idx> res = 0;
    for (Idx i = low; i <= high; ++i) res += func(i);
    return res;
}

template <class Idx, class Func, require_invocable<Func, Idx> = true>
[[nodiscard]] constexpr auto prod(Idx low, Idx high, Func&& func) noexcept(noexcept(func(Idx{}))) {
    assert(low <= high);
    std::invoke_result_t<Func, Idx> res = 1;
    for (Idx i = low; i <= high; ++i) res *= func(i);
    return res;
}

} // namespace utl::math::impl

// ______________________ PUBLIC API ______________________

namespace utl::math {

namespace constants = impl::constants;

using impl::abs;
using impl::sign;
using impl::bsign;
using impl::sqr;
using impl::cube;
using impl::inv;
using impl::heaviside;

using impl::midpoint;
using impl::absdiff;

using impl::pow;
using impl::signpow;

using impl::kronecker_delta;
using impl::levi_civita;

using impl::deg_to_rad;
using impl::rad_to_deg;

using impl::sum;
using impl::prod;

} // namespace utl::math

#endif
#endif // module utl::math






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::mvl
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_mvl.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MVL)
#ifndef UTLHEADERGUARD_MVL
#define UTLHEADERGUARD_MVL

#define UTL_MVL_VERSION_MAJOR 0 // [!] module in early experimental stage,
#define UTL_MVL_VERSION_MINOR 1 //     functional, but needs significant work
#define UTL_MVL_VERSION_PATCH 0 //     to complete and bring up-to-date

// _______________________ INCLUDES _______________________

#include <algorithm>        // swap(), find(), count(), is_sorted(), min_element(),
                            // max_element(), sort(), stable_sort(), min(), max(), remove_if(), copy()
#include <cassert>          // assert() // Note: Perhaps temporary
#include <charconv>         // to_chars()
#include <cmath>            // isfinite()
#include <cstddef>          // size_t, ptrdiff_t, nullptr_t
#include <exception>        // exception
#include <functional>       // reference_wrapper<>, multiplies<>
#include <initializer_list> // initializer_list<>
#include <iomanip>          // setw()
#include <ios>              // right(), boolalpha(), ios::boolalpha
#include <iterator>         // random_access_iterator_tag, reverse_iterator<>
#include <memory>           // unique_ptr<>
#include <numeric>          // accumulate()
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <stdexcept>        // out_of_range, invalid_argument
#include <string>           // string
#include <string_view>      // string_view<>
#include <type_traits>      // conditional_t<>, enable_if_t<>, void_t<>, true_type, false_type, remove_reference_t<>
#include <utility>          // move()
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// [experimental!] This header is still work in progress and likely to undergo significant changes to
// its API & implementation.
//
// This module tries to implement an "unreasonably flexible yet convenient" template for vectors and matrices,
// in order to do so we have to rely heavily on either conditional compilation or automatic code generation
// (or both). There are multiple seemingly viable approaches to it, yet the most of them proved to be either
// unsuitable or unreasonable from the implementation standpoint. Below is a little rundown of implementations
// that were attempted and used/discared for various reasons.
//
// Currently matrix/vector/view/etc code reuse is implemented though a whole bunch of conditional compilation with
// SFINAE, abuse of conditional inheritance, conditional types and constexpr if's. While not perfect, this is
// the best working approach so far. Below is a list of approaches that has been considered & tried:
//
// 1) No code reuse, all classes implemented manually - unrealistically cumbersome, huge code duplication
//
//    => [-] UNSUITABLE APPROACH
//
// 2) Regular OOP with virtual classes - having vtables in lightweight containers is highly undesirable,
//    we can avoid this using CRTP however we run into issues with multiple inheritance (see below)
//
//    => [-] UNSUITABLE APPROACH
//
// 3) CRTP - we unavoidably run into multiple inheritance issues (diamond problem) with concepts like:
//       [mutable-matrix-like] : [const-matrix-like] & [mutable-iterable] where both parents derive from
//       [const-iterable]
//
//    While those issues are resolvable, their handling becomes more and more difficult as parent classes grow larger.
//    A usual solution for this would be virtual inheritance, however it prevents us from calling derived methods inside
//    the base class due to "static downcast via virtual inheritance" rule, which makes 'static_cast<Derived*>(this)'
//    illegal. We can resolve all name collisions manually, however it leads to a code that is absolutely horrific and
//    easy to mess up.
//
//    There is also an issue of extreme boilerplate, since we want to propagate '_types<>' wrapper and the 'Final'
//    return type to all base classes we end up with extremely unwieldy template arguments that make compile errors and
//    refactoring a challenge.
//
//    => [~] SUITABLE, BUT VERY PROBLEMATIC APPROACH
//
// 4) Template-based mixins without CRTP - these allow to add functionality on top of a class, essentially it works
//    like "inverted" CRTP from the code logic POV. While being almost a perfect solution, it doesn't allow us to
//    return objects of derived type from base class methods, which is a requirement for the intended matrix API.
//
//    => [-] UNSUITABLE APPROACH
//
// 5) Macro-based mixins - insert functionality directly in a class, a simple way of doing exactly what we want.
//    "Mixed-in" methods are simply functions implemented in terms of certain "required" methods.
//    Doesn't lead to huge template chains and doesn't have any inheritance at all. The main downside is being
//    preprocessor based, which exposes some identifiers to a global namespace and makes all "connection points"
//    in logic dependant on correct naming inside macros, rather than actual semantical symbols.
//
//    Difficult to work with due to being ignored by the language server, gets increasingly cumbersome as the number of
//    classes grows and still has significant code duplication (vector/matrix * dense/strided/sparse *
//    * container/view/const_view = 18 manually created definitions).
//
//    => [+-] SUITABLE, BUT HEAVILY FLAWED APPROACH
//
// 6) A single class with all parts of API correctly enabled/disabled through SFINAE. Closest thing to a sensible
//    approach with minimal (basically none) code duplication. However it has a number of non-trivial quirks
//    when it comes to conditionally compiling member variables, types & functions (every group has a quirk of
//    its own, functions are the least problematic). Some of such quirks limit possible conditional API (member
//    types suffer from this) or, for example, prevent use of initialization lists on conditionally compiled
//    constructors (big sad).
//
//    A less obvious downside compared to macros, is that it doesn't get handles as well by the language server
//    autocomplete - SFINAE-disabled methods still show up as autocomplete suggestions, even if they get marked
//    as missing immediately upon writing the whole name.
//
//    => [+] DIFFICULT, BUT WORKABLE, BEST APPROACH SO FAR
//
// NOTES ON SPAN:
//
// We can easily implement Matlab-like API to take matrix blocks like this:
//    matrix(Span{0, 10}, 4)
// which would be equivalent to
//    matrix.block(0, 4, 10, 1)
// we only need a thin 'Span' POD with 2 members and a few overloads for 'operator()' that redirect span
// to 'this->block(...)'
//
// NOTES ON NAMING:
//
// Macro naming is a bit of a mess as of now.

// ____________________ IMPLEMENTATION ____________________

namespace utl::mvl {

// ===================
// --- Type Traits ---
// ===================

// Macro for generating type traits with all their boilerplate
//
// While it'd be nice to avoid macro usage alltogether, having a few macros for generating standardized boilerplate
// that gets repeated several dozen times DRASTICALLY improves the maintainability of the whole conditional compilation
// mechanism down the line. They will be later #undef'ed.

#define utl_mvl_define_trait(trait_name_, ...)                                                                         \
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

#define utl_mvl_define_trait_has_binary_op(trait_name_, op_)                                                           \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>>() op_ std::declval<std::decay_t<T>>())

#define utl_mvl_define_trait_has_assignment_op(trait_name_, op_)                                                       \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>&>() op_ std::declval<std::decay_t<T>>())
// for operators like '+=' lhs should be a reference

#define utl_mvl_define_trait_has_unary_op(trait_name_, op_)                                                            \
    utl_mvl_define_trait(trait_name_, op_ std::declval<std::decay_t<T>>())

#define utl_mvl_define_trait_has_member(trait_name_, member_)                                                          \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>>().member_)

#define utl_mvl_define_trait_has_member_type(trait_name_, member_)                                                     \
    utl_mvl_define_trait(trait_name_, std::declval<typename std::decay_t<T>::member_>())

// --- Type traits ---
// -------------------

utl_mvl_define_trait(_has_ostream_output_op, std::declval<std::ostream>() << std::declval<T>());

utl_mvl_define_trait_has_binary_op(_has_binary_op_plus, +);
utl_mvl_define_trait_has_binary_op(_has_binary_op_minus, -);
utl_mvl_define_trait_has_binary_op(_has_binary_op_multiplies, *);
utl_mvl_define_trait_has_binary_op(_has_binary_op_less, <);
utl_mvl_define_trait_has_binary_op(_has_binary_op_greater, >);
utl_mvl_define_trait_has_binary_op(_has_binary_op_equal, ==);

utl_mvl_define_trait_has_assignment_op(_has_assignment_op_plus, +=);
utl_mvl_define_trait_has_assignment_op(_has_assignment_op_minus, -=);
utl_mvl_define_trait_has_assignment_op(_has_assignment_op_multiplies, *=);

utl_mvl_define_trait_has_unary_op(_has_unary_op_plus, +);
utl_mvl_define_trait_has_unary_op(_has_unary_op_minus, -);

utl_mvl_define_trait_has_member(_has_member_i, i);
utl_mvl_define_trait_has_member(_has_member_j, j);
utl_mvl_define_trait_has_member(_has_member_value, value);

utl_mvl_define_trait_has_member(_is_tensor, is_tensor);
utl_mvl_define_trait_has_member(_is_sparse_entry_1d, is_sparse_entry_1d);
utl_mvl_define_trait_has_member(_is_sparse_entry_2d, is_sparse_entry_2d);

// =======================
// --- Stringification ---
// =======================

// Stringification implementation from 'utl::log' module, see its source for more notes.

// --- Internal type traits ---
// ----------------------------

utl_mvl_define_trait(_has_string_append, std::string() += std::declval<T>());
utl_mvl_define_trait(_has_real, std::declval<T>().real());
utl_mvl_define_trait(_has_imag, std::declval<T>().imag());
utl_mvl_define_trait(_has_begin, std::declval<T>().begin());
utl_mvl_define_trait(_has_end, std::declval<T>().end());
utl_mvl_define_trait(_has_input_iter, std::next(std::declval<T>().begin()));
utl_mvl_define_trait(_has_get, std::get<0>(std::declval<T>()));
utl_mvl_define_trait(_has_tuple_size, std::tuple_size<T>::value);
utl_mvl_define_trait(_has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());

// --- Internal utils ---
// ----------------------

template <class>
inline constexpr bool _always_false_v = false;

template <class T>
constexpr int _log_10_ceil(T num) {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

template <class T>
constexpr int _max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, _log_10_ceil(std::numeric_limits<T>::max_exponent10));

template <class T>
constexpr int _max_int_digits = 2 + std::numeric_limits<T>::digits10;

// --- Stringifier ---
// -------------------

// TODO:
// Replace or rethink all of this stringifying stuff, perhaps it would be best to just have a super-simple
// one and let other functions take custom stringifier from 'utl::log'

template <class T>
void _append_stringified(std::string& str, const T& value);

inline void _append_stringified_bool(std::string& str, bool value) { str += value ? "true" : "false"; }

template <class T>
void _append_stringified_integer(std::string& str, T value) {
    std::array<char, _max_int_digits<T>> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (error_code != std::errc())
        throw std::runtime_error(
            "Integer stringification encountered std::to_chars() formatting error while serializing a value.");
    str.append(buffer.data(), number_end_ptr - buffer.data());
}

template <class T>
void _append_stringified_float(std::string& str, T value) {
    std::array<char, _max_float_digits<T>> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (error_code != std::errc())
        throw std::runtime_error(
            "Float stringification encountered std::to_chars() formatting error while serializing a value.");
    str.append(buffer.data(), number_end_ptr - buffer.data());
}

template <class T>
void _append_stringified_complex(std::string& str, T value) {
    _append_stringified_float(str, value.real());
    str += " + ";
    _append_stringified_float(str, value.imag());
    str += " i";
}

template <class T>
void _append_stringified_stringlike(std::string& str, const T& value) {
    str += value;
}

template <class T>
void _append_stringified_string_convertible(std::string& str, const T& value) {
    str += std::string(value);
}

template <class T>
void _append_stringified_array(std::string& str, const T& value) {
    str += "{ ";
    if (value.begin() != value.end())
        for (auto it = value.begin();;) {
            _append_stringified(str, *it);
            if (++it == value.end()) break;
            str += ", ";
        }
    str += " }";
}

template <class Tuple, std::size_t... Idx>
void _append_stringified_tuple_impl(std::string& str, Tuple value, std::index_sequence<Idx...>) {
    ((Idx == 0 ? "" : str += ", ", _append_stringified(str, std::get<Idx>(value))), ...);
}

template <template <class...> class Tuple, class... Args>
void _append_stringified_tuple(std::string& str, const Tuple<Args...>& value) {
    str += "< ";
    _append_stringified_tuple_impl(str, value, std::index_sequence_for<Args...>{});
    str += " >";
}

template <class T>
void _append_stringified_printable(std::string& str, const T& value) {
    str += (std::ostringstream() << value).str();
}

// --- Selector ---
// ----------------

template <class T>
void _append_stringified(std::string& str, const T& value) {
    if constexpr (std::is_same_v<T, bool>) _append_stringified_bool(str, value);
    else if constexpr (std::is_same_v<T, char>) _append_stringified_stringlike(str, value);
    else if constexpr (std::is_integral_v<T>) _append_stringified_integer(str, value);
    else if constexpr (std::is_floating_point_v<T>) _append_stringified_float(str, value);
    else if constexpr (_has_real_v<T> && _has_imag_v<T>) _append_stringified_complex(str, value);
    else if constexpr (std::is_convertible_v<T, std::string_view>) _append_stringified_stringlike(str, value);
    else if constexpr (std::is_convertible_v<T, std::string>) _append_stringified_string_convertible(str, value);
    else if constexpr (_has_begin_v<T> && _has_end_v<T> && _has_input_iter_v<T>) _append_stringified_array(str, value);
    else if constexpr (_has_get_v<T> && _has_tuple_size_v<T>) _append_stringified_tuple(str, value);
    else if constexpr (_has_ostream_insert_v<T>) _append_stringified_printable(str, value);
    else static_assert(_always_false_v<T>, "No valid stringification exists for the type.");
}

// --- Public API ---
// ------------------

template <class... Args>
void append_stringified(std::string& str, Args&&... args) {
    (_append_stringified(str, std::forward<Args>(args)), ...);
}

template <class... Args>
[[nodiscard]] std::string stringify(Args&&... args) {
    std::string buffer;
    append_stringified(buffer, std::forward<Args>(args)...);
    return buffer;
}

// Override "common special cases" that can be improved relative to a generic implementation
[[nodiscard]] inline std::string stringify(int value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(long long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned int value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned long long value) { return std::to_string(value); }

// We wrap stringifying function in functor-class so we can use it a default template callable argument.
// Templates can't infer template parameters from default arguments:
//
//    template<class Func>
//    void do_stuff(Func f = default_f);  // <- CAN'T infer 'Func'
//
//    template<class Func = default_functor>
//    void do_stuff(Func f = Func());     // <- CAN infer 'Func'
//
// which is why the "standard" way of doing it (standard as in used by STL containers) is to use functors as default
// template arguments, if user passes a callable it will override the 'Func' and we get the usual behaviour.
//
template <class T>
struct default_stringifier {
    [[nodiscard]] std::string operator()(const T& value) const { return stringify(value); }
};

// ======================
// --- Codegen Macros ---
// ======================

#define utl_mvl_assert(condition_) assert(condition_)
// if (!__VA_ARGS__) throw std::runtime_error("Failed assert on line " + std::to_string(__LINE__))

// ========================
// --- Helper Functions ---
// ========================

// Shortcut for lambda-type-based SFINAE.
//
// Callables in this module are usually takes as a template type since 'std::function<>' introduces very significant
// overhead with its type erasure. With template args all lambdas and functors can be nicely inlined, however we lose
// the ability to overload functions that take callable the way we could with 'std::function<>'.
//
// As a workaround we can use SFINAE to reject "overloads" tha don't have a particular signature, effectively achieving
// the behaviour we need.
template <class FuncType, class Signature>
using _has_signature_enable_if = std::enable_if_t<std::is_convertible_v<FuncType, std::function<Signature>>, bool>;

template <class T>
[[nodiscard]] std::unique_ptr<T[]> _make_unique_ptr_array(size_t size) {
    return std::unique_ptr<T[]>(new T[size]);
}

// Marker for unreachable code
[[noreturn]] inline void _unreachable() {
// (Implementation from https://en.cppreference.com/w/cpp/utility/unreachable)
// Use compiler specific extensions if possible.
// Even if no extension is used, undefined behavior is still raised by
// an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// =======================
// --- Utility Classes ---
// =======================

// Wrapper that allows passing standard set of member types down the CRTP chain
// (types have to be passed through template args one way or another. Trying to access
// 'derived_type::value_type' from the CRTP base class will fail, since in order for it to be available
// the derived class has to be "implemented" which can only happen after the base class is "implemented",
// which requires the type => a logical loop. So we wrap all the types in a class a pass them down
// the chain a "pack with everything" type of deal. This pack then gets "unwrapped" with
// '_utl_storage_define_types' macro in every class => we have all the member types defined).
//
// (!) Since CRTP approach has been deprecated in favor of mixins, the original reasoning is no longer
// accurate, however it is this a useful class whenever a pack of types has to be passed through a
// template (like, for example, in iterator implementation)
template <class T>
struct _types {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
};

// A minimal equivalent of the once proposed 'std::observer_ptr<>'.
// Used in views to allow observer pointers with the same interface as 'std::unique_ptr',
// which means a generic '.data()` can be implemented without having to make a separate version for views and containers
template <class T>
class _observer_ptr {
public:
    using element_type = T;

private:
    element_type* _data = nullptr;

public:
    // - Constructors -
    constexpr _observer_ptr() noexcept = default;
    constexpr _observer_ptr(std::nullptr_t) noexcept {}
    explicit _observer_ptr(element_type* ptr) : _data(ptr) {}

    _observer_ptr& operator=(element_type* ptr) {
        this->_data = ptr;
        return *this;
    }

    // - Interface -
    [[nodiscard]] constexpr element_type* get() const noexcept { return _data; }
};

// =================
// --- Iterators ---
// =================

// Iterator template for 1D iterator over a tensor.
// Reduces code duplication for const & non-const variants by bundling some conditional logic.
// Uses 'operator[]' of 'ParentPointer' for all logic, thus allowing arbitrary tensors that
// don't have to be contiguous or even ordered in memory.
template <class ParentPointer, class Types, bool is_const_iter = false>
class _flat_iterator {
    using parent_pointer = ParentPointer;

public:
    // Iterator reqs: [General]
    // Contains member types      =>
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename Types::difference_type;
    using value_type        = typename Types::value_type;
    using pointer = typename std::conditional_t<is_const_iter, typename Types::const_pointer, typename Types::pointer>;
    using reference =
        typename std::conditional_t<is_const_iter, typename Types::const_reference, typename Types::reference>;

    // Iterator reqs: [General]
    // Constructible
    _flat_iterator(parent_pointer parent, difference_type idx) : _parent(parent), _idx(idx) {}
    // Copy-constructible => by default
    // Copy-assignable    => by default
    // Destructible       => by default
    // Swappable
    friend void swap(_flat_iterator& lhs, _flat_iterator& rhs) { std::swap(lhs._idx, rhs._idx); }

    // Iterator reqs: [General]
    // Can be incremented (prefix & postfix)
    _flat_iterator& operator++() {
        ++_idx;
        return *this;
    } // prefix
    _flat_iterator operator++(int) {
        _flat_iterator temp = *this;
        ++_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Input iterator]
    // Supports equality/inequality comparisons
    friend bool operator==(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx == it2._idx; };
    friend bool operator!=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx != it2._idx; };
    // Can be dereferenced as an rvalue
    reference   operator*() const { return _parent->operator[](_idx); }
    pointer     operator->() const { return &_parent->operator[](_idx); }

    // Iterator reqs: [Output iterator]
    // Can be dereferenced as an lvalue (only for mutable iterator types) => not needed

    // Iterator reqs: [Forward iterator]
    // Default-constructible
    _flat_iterator() : _parent(nullptr), _idx(0) {}
    // "Multi-pass" - dereferencing & incrementing does not affects dereferenceability => satisfied

    // Iterator reqs: [Bidirectional iterator]
    // Can be decremented
    _flat_iterator& operator--() {
        --_idx;
        return *this;
    } // prefix
    _flat_iterator operator--(int) {
        _flat_iterator temp = *this;
        --_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Random Access iterator]
    // See: https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
    // Supports arithmetic operators: it + n, n + it, it - n, it1 - it2
    friend _flat_iterator operator+(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator+(difference_type diff, const _flat_iterator& it) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator-(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx - diff);
    }
    friend difference_type operator-(const _flat_iterator& it1, const _flat_iterator& it2) {
        return it1._idx - it2._idx;
    }
    // Supports inequality comparisons (<, >, <= and >=) between iterators
    friend bool     operator<(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx < it2._idx; }
    friend bool     operator>(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx > it2._idx; }
    friend bool     operator<=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx <= it2._idx; }
    friend bool     operator>=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx >= it2._idx; }
    // Standard assumption: Both iterators are from the same container => no need to compare '_parent'
    // Supports compound assignment operations += and -=
    _flat_iterator& operator+=(difference_type diff) {
        _idx += diff;
        return *this;
    }
    _flat_iterator& operator-=(difference_type diff) {
        _idx -= diff;
        return *this;
    }
    // Supports offset dereference operator ([])
    reference operator[](difference_type diff) const { return _parent->operator[](_idx + diff); }

private:
    parent_pointer  _parent;
    difference_type _idx; // not size_type because we have to use it in 'difference_type' operations most of the time
};

// ======================================
// --- Sparse Matrix Pairs & Triplets ---
// ======================================

// Note:
// All sparse entries and multi-dimensional indices can be sorted lexicographically

template <class T>
struct SparseEntry1D {
    size_t i;
    T      value;

    constexpr static bool is_sparse_entry_1d = true;

    [[nodiscard]] bool operator<(const SparseEntry1D& other) const noexcept { return this->i < other.i; }
    [[nodiscard]] bool operator>(const SparseEntry1D& other) const noexcept { return this->i > other.i; }
};

template <class T>
struct SparseEntry2D {
    size_t i;
    size_t j;
    T      value;

    constexpr static bool is_sparse_entry_2d = true;

    [[nodiscard]] bool operator<(const SparseEntry2D& other) const noexcept {
        return (this->i < other.i) || (this->j < other.j);
    }
    [[nodiscard]] bool operator>(const SparseEntry2D& other) const noexcept {
        return (this->i > other.i) || (this->j > other.j);
    }
};

struct Index2D {
    size_t i;
    size_t j;

    [[nodiscard]] bool operator<(const Index2D& other) const noexcept {
        return (this->i < other.i) || (this->j < other.j);
    }
    [[nodiscard]] bool operator>(const Index2D& other) const noexcept {
        return (this->i > other.i) || (this->j > other.j);
    }
    [[nodiscard]] bool operator==(const Index2D& other) const noexcept {
        return (this->i == other.i) && (this->j == other.j);
    }
};

// Functions that take 2 sparse entries '{ i, j, v1 }' and '{ i, j, v2 }' and return '{ i, j, op(v1, v2) }'.
// Same thing for 1D sparse entries. LHS/RHS index correctness only gets checked in debug.
//
// Declaring these functions saves us from having to duplicate implementations of 'apply_binary_operator()'
// for 1D and 2D sparse entries that have different number of constructor args. Instead, those cases will
// be handled by '_apply_binary_op_to_sparse_entry()' that is SFINAE-overloaded to work with both.
//
// Implementations use perfect forwarding for everything and do the "overloading" through SFINAE with type traits.
//
// In reality 'op' doesn't ever get forwarded as an r-value but since are doing things generic why not forward it
// properly as well. This is also somewhat ugly, but it works well enough and there isn't much reason to rewrite
// this with some new abstraction.
//
template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<L> = true, _is_sparse_entry_1d_enable_if<R> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entries(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.i == right.i);
    return {left.i, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<L> = true, _is_sparse_entry_2d_enable_if<R> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entries(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.i == right.i);
    utl_mvl_assert(left.j == right.j);
    return {left.i, left.j, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<L> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entry_and_value(L&& left, R&& right_value, Op&& op) {
    return {left.i, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right_value))};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<L> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entry_and_value(L&& left, R&& right_value, Op&& op) {
    return {left.i, left.j, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right_value))};
}

template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<R> = true>
std::decay_t<R> _apply_binary_op_to_value_and_sparse_entry(L&& left_value, R&& right, Op&& op) {
    return {right.i, std::forward<Op>(op)(std::forward<L>(left_value), std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<R> = true>
std::decay_t<R> _apply_binary_op_to_value_and_sparse_entry(L&& left_value, R&& right, Op&& op) {
    return {right.i, right.j, std::forward<Op>(op)(std::forward<L>(left_value), std::forward<R>(right).value)};
}

// TODO:
// Fix binary ops, figure out what the hell did I even do here

// =============
// --- Enums ---
// =============

// Parameter enums
//
// Their combination specifies compiled tensor API.
enum class Dimension { VECTOR, MATRIX };
enum class Type { DENSE, STRIDED, SPARSE };
enum class Ownership { CONTAINER, VIEW, CONST_VIEW };

// Config enums
//
// They specify conditional logic for a tensor.
// Combination of config & parameter enums fully defines a GenericTensor.
enum class Checking { NONE, BOUNDS };
enum class Layout { /* 1D */ FLAT, /* 2D */ RC, CR, /* Other */ SPARSE };

// Shortcut template used to deduce type of '_data' based on tensor 'ownership' parameter
template <Ownership ownership, class ContainerResult, class ViewResult, class ConstViewResult>
using _choose_based_on_ownership =
    std::conditional_t<ownership == Ownership::CONTAINER, ContainerResult,
                       std::conditional_t<ownership == Ownership::VIEW, ViewResult, ConstViewResult>>;

// Shortcut template used to check that both arguments are tensors
// that have the same value type, mostly used in binary operators
// for nicer error messages and to prevent accidental conversions
template <class L, class R>
using _are_tensors_with_same_value_type_enable_if =
    std::enable_if_t<_is_tensor_v<L> && _is_tensor_v<R> &&
                         std::is_same_v<typename std::decay_t<L>::value_type, typename std::decay_t<R>::value_type>,
                     bool>;

// =====================================
// --- Boilerplate generation macros ---
// =====================================

// Macros used to pass around unwieldy chains of tensor template arguments.
//
#define utl_mvl_tensor_arg_defs                                                                                        \
    class T, Dimension _dimension, Type _type, Ownership _ownership, Checking _checking, Layout _layout

#define utl_mvl_tensor_arg_vals T, _dimension, _type, _ownership, _checking, _layout

// Incredibly important macros used for conditional compilation of member functions.
// They automatically create the boilerplate that makes member functions dependant on the template parameters,
// which is necessary for conditional compilation and "forward" to a 'enable_if' condition.
//
// 'utl_mvl_require' is intended to be used inside template methods (template in a sense that they have other template
// args besides this conditional compilation boilerplate which technically makes all methods template).
//
// 'utl_mvl_reqs' is used when we want only conditional compilation (aka majority of cases) and cuts down on
// boilerplate even more.
//
#define utl_mvl_require(condition_)                                                                                    \
    class value_type = T, Dimension dimension = _dimension, Type type = _type, Ownership ownership = _ownership,       \
          Checking checking = _checking, Layout layout = _layout, std::enable_if_t<condition_, bool> = true

#define utl_mvl_reqs(condition_) template <utl_mvl_require(condition_)>

// A somewhat scuffed version of trait-defining macro used to create SFINAE-restrictions
// on tensor params in free functions. Only supports trivial conditions of the form
// '<parameter> [==][!=] <value>'. Perhaps there is a better way of doing it, but I'm not yet sure.
//
// Used mostly to restrict linear algebra operations for sparse/nonsparse so we can get
// a "SFINAE-driven overloading" on operators that take both arguments with perfect forwarding,
// which means they can't be overloaded in a regular way
//
#define utl_mvl_define_tensor_param_restriction(trait_name_, expr_)                                                    \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = (std::decay_t<T>::params::expr_);                                                 \
                                                                                                                       \
    template <class T>                                                                                                 \
    using trait_name_##_enable_if = std::enable_if_t<trait_name_##_v<T>, bool>;

utl_mvl_define_tensor_param_restriction(_is_sparse_tensor, type == Type::SPARSE);
utl_mvl_define_tensor_param_restriction(_is_nonsparse_tensor, type != Type::SPARSE);
utl_mvl_define_tensor_param_restriction(_is_matrix_tensor, dimension == Dimension::MATRIX);

// ===========================
// --- Data Member Classes ---
// ===========================

// Unlike class method, member values can't be templated, which prevents us from using regular 'enable_if_t' SFINAE
// for their conditional compilation. The (seemingly) best workaround to compile members conditionally is to inherit
// 'std::conditional<T, EmptyClass>' where 'T' is a "dummy" class with the sole purpose of having data members to
// inherit. This does not introduce virtualization (which is good, that how we want it).

template <int id>
struct _nothing {};

template <utl_mvl_tensor_arg_defs>
class _2d_extents {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _rows = 0;
    size_type _cols = 0;
};

template <utl_mvl_tensor_arg_defs>
class _2d_strides {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _row_stride = 0;
    size_type _col_stride = 0;
};

template <utl_mvl_tensor_arg_defs>
struct _2d_dense_data {
private:
    using value_type = typename _types<T>::value_type;
    using _data_t    = _choose_based_on_ownership<_ownership, std::unique_ptr<value_type[]>, _observer_ptr<value_type>,
                                               _observer_ptr<const value_type>>;

public:
    _data_t _data;
};

template <utl_mvl_tensor_arg_defs>
struct _2d_sparse_data {
private:
    using value_type = typename _types<T>::value_type;
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using triplet_type = _triplet_t;

    std::vector<triplet_type> _data;
};

// ===================
// --- Tensor Type ---
// ===================

template <utl_mvl_tensor_arg_defs>
class GenericTensor
    // Conditionally compile member variables through inheritance
    : public std::conditional_t<_dimension == Dimension::MATRIX, _2d_extents<utl_mvl_tensor_arg_vals>, _nothing<1>>,
      public std::conditional_t<_dimension == Dimension::MATRIX && _type == Type::STRIDED,
                                _2d_strides<utl_mvl_tensor_arg_vals>, _nothing<2>>,
      public std::conditional_t<_type == Type::DENSE || _type == Type::STRIDED, _2d_dense_data<utl_mvl_tensor_arg_vals>,
                                _nothing<3>>,
      public std::conditional_t<_type == Type::SPARSE, _2d_sparse_data<utl_mvl_tensor_arg_vals>, _nothing<4>>
// > After this point no non-static member variables will be introduced
{
    // --- Parameter reflection ---
    // ----------------------------
public:
    struct params {
        constexpr static auto dimension = _dimension;
        constexpr static auto type      = _type;
        constexpr static auto ownership = _ownership;
        constexpr static auto checking  = _checking;
        constexpr static auto layout    = _layout;

        // Prevent impossible layouts
        static_assert((dimension == Dimension::VECTOR) == (layout == Layout::FLAT), "Flat layout <=> matrix is 1D.");
        static_assert((type == Type::SPARSE) == (layout == Layout::SPARSE), "Sparse layout <=> matrix is sparse.");
    };

    constexpr static bool is_tensor = true;

    // --- Member types ---
    // --------------------
private:
    using _type_wrapper = _types<T>;

public:
    using self            = GenericTensor;
    using value_type      = typename _type_wrapper::value_type;
    using size_type       = typename _type_wrapper::size_type;
    using difference_type = typename _type_wrapper::difference_type;
    using reference       = typename _type_wrapper::reference;
    using const_reference = typename _type_wrapper::const_reference;
    using pointer         = typename _type_wrapper::pointer;
    using const_pointer   = typename _type_wrapper::const_pointer;

    using owning_reflection = GenericTensor<value_type, params::dimension, params::type, Ownership::CONTAINER,
                                            params::checking, params::layout>;
    // container type corresponding to 'self', this is the return type of algebraic operations on a tensor

    // --- Iterators ---
    // -----------------
public:
    using const_iterator         = _flat_iterator<const self*, _type_wrapper, true>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using iterator = std::conditional_t<_ownership != Ownership::CONST_VIEW, _flat_iterator<self*, _type_wrapper>,
                                        const_iterator>; // for const views 'iterator' is the same as 'const_iterator'
    using reverse_iterator = std::reverse_iterator<iterator>;

    [[nodiscard]] const_iterator cbegin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(this, this->size()); }
    [[nodiscard]] const_iterator begin() const { return this->cbegin(); }
    [[nodiscard]] const_iterator end() const { return this->cend(); }

    [[nodiscard]] const_reverse_iterator crbegin() const { return const_reverse_iterator(this->cend()); }
    [[nodiscard]] const_reverse_iterator crend() const { return const_reverse_iterator(this->cbegin()); }
    [[nodiscard]] const_reverse_iterator rbegin() const { return this->crbegin(); }
    [[nodiscard]] const_reverse_iterator rend() const { return this->crend(); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator begin() { return iterator(this, 0); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator end() { return iterator(this, this->size()); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rbegin() { return reverse_iterator(this->end()); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rend() { return reverse_iterator(this->begin()); }

    // --- Basic getters ---
    // ---------------------
public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type size() const noexcept { return this->rows() * this->cols(); }

    utl_mvl_reqs(type == Type::SPARSE)
    [[nodiscard]] size_type size() const noexcept { return this->_data.size(); }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type rows() const noexcept { return this->_rows; }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type cols() const noexcept { return this->_cols; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type row_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 0;
        if constexpr (self::params::layout == Layout::CR) return 1;
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type col_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 1;
        if constexpr (self::params::layout == Layout::CR) return 0;
        _unreachable();
    }
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type row_stride() const noexcept { return this->_row_stride; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type col_stride() const noexcept { return this->_col_stride; }

    utl_mvl_reqs(type == Type::DENSE || type == Type::STRIDED)
    [[nodiscard]] const_pointer data() const noexcept { return this->_data.get(); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] pointer data() noexcept { return this->_data.get(); }

    [[nodiscard]] bool empty() const noexcept { return (this->size() == 0); }

    // --- Advanced getters ---
    // ------------------------
    utl_mvl_reqs(_has_binary_op_equal<value_type>::value)
    [[nodiscard]] bool contains(const_reference value) const {
        return std::find(this->cbegin(), this->cend(), value) != this->cend();
    }

    utl_mvl_reqs(_has_binary_op_equal<value_type>::value)
    [[nodiscard]] size_type count(const_reference value) const {
        return std::count(this->cbegin(), this->cend(), value);
    }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] bool is_sorted() const { return std::is_sorted(this->cbegin(), this->cend()); }

    template <class Compare>
    [[nodiscard]] bool is_sorted(Compare cmp) const {
        return std::is_sorted(this->cbegin(), this->cend(), cmp);
    }

    [[nodiscard]] std::vector<value_type> to_std_vector() const { return std::vector(this->cbegin(), this->cend()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    self transposed() const {
        self res(this->cols(), this->rows());
        this->for_each([&](const value_type& element, size_type i, size_type j) { res(j, i) = element; });
        return res;
    }

    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self clone() const { return *this; }

    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self move() & { return std::move(*this); }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout>
    [[nodiscard]] bool
    compare_contents(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                         other_checking, other_layout>& other) const {
        // Surface-level checks
        if ((this->rows() != other.rows()) || (this->cols() != other.cols())) return false;
        // Compare while respecting sparsity
        constexpr bool is_sparse_l = (self::params::type == Type::SPARSE);
        constexpr bool is_sparse_r = (std::remove_reference_t<decltype(other)>::params::type == Type::SPARSE);
        // Same sparsity comparison
        if constexpr (is_sparse_l == is_sparse_r) {
            return this->size() == other.size() &&
                   this->true_for_all([&](const_reference e, size_type i, size_type j) { return e == other(i, j); });
        }
        // Different sparsity comparison
        // TODO: Impl here and use .all_of() OR .any_of()
        return true;
    }

    // --- Indexation ---
    // ------------------
public:
    // Vector API
    [[nodiscard]] const_reference front() const { return this->operator[](0); }
    [[nodiscard]] const_reference back() const { return this->operator[](this->size() - 1); }
    [[nodiscard]] reference       front() { return this->operator[](0); }
    [[nodiscard]] reference       back() { return this->operator[](this->size() - 1); }

private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type _get_memory_offset_strided_impl(size_type idx, size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return idx * this->col_stride() + this->row_stride() * i;
        if constexpr (self::params::layout == Layout::CR) return idx * this->row_stride() + this->col_stride() * j;
        _unreachable();
    }

public:
    utl_mvl_reqs(type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return idx;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        return this->get_idx_of_ij(i, j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        const auto ij = this->get_ij_of_idx(idx);
        return _get_memory_offset_strided_impl(idx, ij.i, ij.j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        const auto idx = this->get_idx_of_ij(i, j);
        return _get_memory_offset_strided_impl(idx, i, j);
    }

public:
    // - Flat indexation -
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
                 (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator[](size_type idx) { return this->data()[this->get_memory_offset_of_idx(idx)]; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator[](size_type idx) const {
        return this->data()[this->get_memory_offset_of_idx(idx)];
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] reference operator[](size_type idx) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    utl_mvl_reqs(dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] const_reference operator[](size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    // - 2D indexation -
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
                 (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }

    // --- Index conversions ---
    // -------------------------

    // - Bound checking -
private:
    void _bound_check_idx(size_type idx) const {
        if (idx >= this->size())
            throw std::out_of_range(
                stringify("idx (which is ", idx, ") >= this->size() (which is ", this->size(), ")"));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    void _bound_check_ij(size_type i, size_type j) const {
        if (i >= this->rows())
            throw std::out_of_range(stringify("i (which is ", i, ") >= this->rows() (which is ", this->rows(), ")"));
        else if (j >= this->cols())
            throw std::out_of_range(stringify("j (which is ", j, ") >= this->cols() (which is ", this->cols(), ")"));
    }

    // - Dense & strided implementations -
private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type _unchecked_get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return i * this->cols() + j;
        if constexpr (self::params::layout == Layout::CR) return j * this->rows() + i;
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D _unchecked_get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::layout == Layout::RC) return {idx / this->cols(), idx % this->cols()};
        if constexpr (self::params::layout == Layout::CR) return {idx % this->rows(), idx / this->rows()};
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    [[nodiscard]] size_type _total_allocated_size() const noexcept {
        // Note 1: Allocated size of the strided matrix is NOT equal to .size() (which is same as rows * cols)
        // This is due to all the padding between the actual elements
        // Note 2: The question of whether .size() should return the number of 'strided' elements or the number
        // of actually allocated elements is rather perplexing, while the second option is more "correct" its
        // usefulness is dubious at bets, while the first one does in fact allow convenient 1D iteration over
        // all the elements. Option 1 also provides an API consistent with strided views, which is why it ended
        // up being the one chosen
        //
        // This method returns a true allocated size
        if constexpr (self::params::layout == Layout::RC)
            return (this->rows() - 1) * this->row_stride() + this->rows() * this->cols() * this->col_stride();
        if constexpr (self::params::layout == Layout::CR)
            return (this->cols() - 1) * this->col_stride() + this->rows() * this->cols() * this->row_stride();
        _unreachable();
    }

public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_ij(i, j);
        return _unchecked_get_idx_of_ij(i, j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return _unchecked_get_ij_of_idx(idx);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_major() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->rows();
        if constexpr (self::params::layout == Layout::CR) return this->cols();
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_minor() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->cols();
        if constexpr (self::params::layout == Layout::CR) return this->rows();
        _unreachable();
    }

    // - Sparse implementations -
private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type _search_ij(size_type i, size_type j) const noexcept {
        // Returns this->size() if {i, j} wasn't found.
        // Linear search for small .size() (more efficient fue to prediction and cache locality)
        if (true) {
            for (size_type idx = 0; idx < this->size(); ++idx)
                if (this->_data[idx].i == i && this->_data[idx].j == j) return idx;
            return this->size();
        }
        // TODO: Binary search for larger .size() (N(log2(size)) instead of N(size) asymptotically)
    }

public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        const size_type idx = this->_search_ij(i, j);
        // Return this->size() if {i, j} wasn't found. Throw with bound checking.
        if constexpr (self::params::checking == Checking::BOUNDS)
            if (idx == this->size())
                throw std::out_of_range(stringify("Index { ", i, ", ", j, "} in not a part of sparse matrix"));
        return idx;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return Index2D{this->_data[idx].i, this->_data[idx].j};
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] bool contains_index(size_type i, size_type j) const noexcept {
        return this->_search_ij(i, j) != this->size();
    }

    // --- Reductions ---
    // ------------------
    utl_mvl_reqs(_has_binary_op_plus<value_type>::value)
    [[nodiscard]] value_type sum() const { return std::accumulate(this->cbegin(), this->cend(), value_type()); }

    utl_mvl_reqs(_has_binary_op_multiplies<value_type>::value)
    [[nodiscard]] value_type product() const {
        return std::accumulate(this->cbegin(), this->cend(), value_type(), std::multiplies<value_type>());
    }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] value_type min() const { return *std::min_element(this->cbegin(), this->cend()); }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] value_type max() const { return *std::max_element(this->cbegin(), this->cend()); }

    // --- Predicate operations ---
    // ----------------------------
    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion
        // This is just as fast and ensures looping only over existing elements in non-dense matrices
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            if (predicate(this->operator[](idx), ij.i, ij.j)) return true;
        }
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inverse_predicate = [&](const_reference e) -> bool { return !predicate(e); };
        return !this->true_for_any(inverse_predicate);
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inverse_predicate = [&](const_reference e, size_type idx) -> bool { return !predicate(e, idx); };
        return !this->true_for_any(inverse_predicate);
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        // We can reuse .true_for_any() with inverted predicate due to following conjecture:
        // FOR_ALL (predicate)  ~  ! FOR_ANY (!predicate)
        auto inverse_predicate = [&](const_reference e, size_type i, size_type j) -> bool {
            return !predicate(e, i, j);
        };
        return !this->true_for_any(inverse_predicate);
    }

    // --- Const algorithms ---
    // ------------------------
    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference)> = true>
    const self& for_each(FuncType func) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference, size_type)> = true>
    const self& for_each(FuncType func) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    const self& for_each(FuncType func) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion.
        // This is just as fast and ensures looping only over existing elements in non-dense matrices.
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    // --- Mutating algorithms ---
    // ---------------------------
    template <class FuncType, _has_signature_enable_if<FuncType, void(reference)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(reference, size_type)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(const_reference)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(elem); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(const_reference, size_type)> = true,
              utl_mvl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(elem, i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType,
              _has_signature_enable_if<FuncType, value_type(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(elem, i, j); };
        return this->for_each(func_wrapper);
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    self& fill(const_reference value) {
        for (size_type idx = 0; idx < this->size(); ++idx) this->operator[](idx) = value;
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type()> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(size_type)> = true,
              utl_mvl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(i, j); };
        return this->for_each(func_wrapper);
    }

    template <class Compare, utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& sort(Compare cmp) {
        std::sort(this->begin(), this->end(), cmp);
        return *this;
    }
    template <class Compare, utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& stable_sort(Compare cmp) {
        std::stable_sort(this->begin(), this->end(), cmp);
        return *this;
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && _has_binary_op_less<value_type>::value)
    self& sort() {
        std::sort(this->begin(), this->end());
        return *this;
    }
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && _has_binary_op_less<value_type>::value)
    self& stable_sort() {
        std::stable_sort(this->begin(), this->end());
        return *this;
    }

    // --- Sparse Subviews ---
    // -----------------------

    // - Const views -
private:
    using _cref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<const value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_const_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE,
                                                 Ownership::CONST_VIEW, self::params::checking, Layout::SPARSE>;

    template <class UnaryPredicate, _has_signature_enable_if<UnaryPredicate, bool(const_reference)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate, _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate,
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        _cref_triplet_array triplets;
        const auto          add_triplet_if_predicate = [&](const_reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] sparse_const_view_type diagonal() const {
        // Sparse matrices have no better way of getting a diagonal than filtering (i ==j)
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        }
        // Non-sparse matrices can just iterate over diagonal directly
        else {
            const size_type     min_size = std::min(this->rows(), this->cols());
            _cref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // - Mutable views -
private:
    using _ref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE, Ownership::VIEW,
                                           self::params::checking, Layout::SPARSE>;

    template <class UnaryPredicate, utl_mvl_require(ownership != Ownership::CONST_VIEW),
              _has_signature_enable_if<UnaryPredicate, bool(const_reference)> = true>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate, utl_mvl_require(ownership != Ownership::CONST_VIEW),
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type)> = true>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate,
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        // This method implements actual filtering, others just forward predicates to it
        _ref_triplet_array triplets;
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        const auto         add_triplet_if_predicate = [&](reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] sparse_view_type diagonal() {
        /* Sparse matrices have no better way of getting a diagonal than filtering (i == j) */
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        } /* Non-sparse matrices can just iterate over diagonal directly */
        else {
            const size_type    min_size = std::min(this->rows(), this->cols());
            _ref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // --- Block Subviews ---
    // ----------------------
public:
    // - Const views -
    using block_const_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_const_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::CONST_VIEW,
                                         self::params::checking, self::params::layout>>;

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _cref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](const_reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_const_view_type(block_rows, block_cols, std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type row(size_type i) const { return this->block(i, 0, 1, this->cols()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type col(size_type j) const { return this->block(0, j, this->rows(), 1); }

    // - Mutable views -
    using block_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::VIEW,
                                         self::params::checking, self::params::layout>>;

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _ref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_view_type(block_rows, block_cols, std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type row(size_type i) { return this->block(i, 0, 1, this->cols()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type col(size_type j) { return this->block(0, j, this->rows(), 1); }

    // --- Sparse operations ---
    // -------------------------

private:
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using sparse_entry_type = _triplet_t;

    utl_mvl_reqs(type == Type::SPARSE)
    [[nodiscard]] const std::vector<sparse_entry_type>& entries() const noexcept { return this->_data; }

    utl_mvl_reqs(type == Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] std::vector<sparse_entry_type>& entries() noexcept { return this->_data; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& insert_triplets(const std::vector<sparse_entry_type>& triplets) {
        // Bulk-insert triplets and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data.insert(this->_data.end(), triplets.begin(), triplets.end());
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& rewrite_triplets(std::vector<sparse_entry_type>&& triplets) {
        // Move-construct all triplets at once and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data = std::move(triplets);
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& erase_triplets(std::vector<Index2D> indices) {
        // Erase triplets with {i, j} from 'indices' using the fact that both
        // 'indices' and triplets are sorted. We can scan through triplets once
        // while advancing 'cursor' when 'indices[cursor]' gets deleted, which
        // result in all necessary triplets being marked for erasure in order.
        std::sort(indices.begin(), indices.end());
        std::size_t cursor = 0;

        const auto erase_condition = [&](const sparse_entry_type& triplet) -> bool {
            /* Stop erasing once all target indices are handled */
            if (cursor == indices.size()) return false;
            if (indices[cursor].i == triplet.i && indices[cursor].j == triplet.j) {
                ++cursor;
                return true;
            }
            return false;
        };

        const auto iter = std::remove_if(this->_data.begin(), this->_data.end(), erase_condition);
        this->_data.erase(iter, this->_data.end());

        // Re-sort triplets just in case
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    // --- Constructors ---
    // --------------------

    // - Matrix -
public:
    // Rule of five:
    // copy-ctor       - [+] (deduced from copy-assignment)
    // move-ctor       - [+] (= default)
    // copy-assignment - [+] (custom for dense/strided, same as default for sparse)
    // move-assignment - [+] (= default)
    // destructor      - [+] (= default)

    // Move-ctor is default for all types
    GenericTensor(self&& other) noexcept = default;

    // Move-assignment is default for all types
    self& operator=(self&& other) noexcept = default;

    // Copy-ctor is deduced from assignment operator
    GenericTensor(const self& other) { *this = other; }

    // Copy-assignment
    self& operator=(const self& other) {
        // Note: copy-assignment operator CANNOT be templated, it has to be implemented with 'if constexpr'
        this->_rows = other.rows();
        this->_cols = other.cols();
        if constexpr (self::params::type == Type::DENSE) {
            this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::STRIDED) {
            this->_row_stride = other.row_stride();
            this->_col_stride = other.col_stride();
            this->_data       = std::move(_make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::SPARSE) { this->_data = other._data; }
        return *this;
    }

    // Default-ctor (containers)
    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    GenericTensor() noexcept {}

    // Default-ctor (views)
    utl_mvl_reqs(ownership != Ownership::CONTAINER)
    GenericTensor() noexcept = delete;

    // Copy-assignment over the config boundaries
    // We can change checking config, copy from matrices with different layouts,
    // copy from views and even matrices of other types
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to dense works, all elements that weren't in the sparse matrix remain default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        // Not quite sure whether swapping strides when changing layouts like this is okay,
        // but it seems to be correct
        if constexpr (self::params::layout != other_layout) std::swap(this->_row_stride, this->_cols_stride);
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to strided works, all elements that weren't in the sparse matrix remain
        // default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::SPARSE &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        std::vector<sparse_entry_type> triplets;

        // Other sparse matrices can be trivially copied
        if constexpr (other_type == Type::SPARSE) {
            triplets.reserve(other.size());
            other.for_each([&](const value_type& elem, size_type i, size_type j) { triplets.push_back({i, j, elem}); });
        }
        // Non-sparse matrices are filtered by non-default-initialized-elements to construct a sparse subset
        else {
            other.for_each([&](const_reference elem, size_type i, size_type j) {
                if (elem != value_type()) triplets.push_back({i, j, elem});
            });
        }

        this->rewrite_triplets(std::move(triplets));
        return *this;
    }

    // Copy-ctor over the config boundaries (deduced from assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership, other_checking,
                                      other_layout>& other) {
        *this = other;
    }

    // Move-assignment over config boundaries
    // Note: Unlike copying, we can't change layout, only checking config
    // Also 'other' can no longer be a view or have a different type
    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = std::move(other._data);
        return *this;
    }

    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::SPARSE &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    // Move-ctor over the config boundaries (deduced from move-assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, other_type, other_ownership, other_checking,
                                other_layout>&& other) {
        *this = std::move(other);
    }

    // Init-with-value
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, const_reference value = value_type()) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <class FuncType, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                                              ownership == Ownership::CONTAINER)>
    explicit GenericTensor(size_type rows, size_type cols, FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init) {
        this->_rows = init.size();
        this->_cols = (*init.begin()).size();
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if (static_cast<size_type>((*row_it).end() - (*row_it).begin()) != this->cols())
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) noexcept {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(decltype(this->_data)(data_ptr));
    }

    // - Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Const Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, const_pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                              ownership == Ownership::CONST_VIEW)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                      other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Strided Matrix -

    // Init-with-value
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_reference value = value_type()) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <class FuncType, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                                              ownership == Ownership::CONTAINER)>
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init, size_type row_stride,
                  size_type col_stride) {
        this->_rows       = init.size();
        this->_cols       = (*init.begin()).size();
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if ((*row_it).end() - (*row_it).begin() != this->_cols)
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) noexcept {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = std::move(decltype(this->_data)(data_ptr));
    }

    // - Strided Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Const Strided Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                              ownership == Ownership::CONST_VIEW)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                      other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Sparse Matrix / Sparse Matrix View / Sparse Matrix Const View -

    // Init-from-data (copy)
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, const std::vector<sparse_entry_type>& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->insert_triplets(std::move(data));
    }

    // Init-from-data (move)
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, std::vector<sparse_entry_type>&& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->rewrite_triplets(std::move(data));
    }
};

// ===========================
// --- Predefined Typedefs ---
// ===========================

constexpr auto _default_checking        = Checking::NONE;
constexpr auto _default_layout_dense_2d = Layout::RC;

// - Dense 2D -
template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using Matrix = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using MatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::VIEW, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstMatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONST_VIEW, checking, layout>;

// - Strided 2D -
template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrix = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONTAINER, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrixView = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::VIEW, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstStridedMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONST_VIEW, checking, layout>;

// - Sparse 2D -
template <class T, Checking checking = _default_checking>
using SparseMatrix = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONTAINER, checking, Layout::SPARSE>;

template <class T, Checking checking = _default_checking>
using SparseMatrixView = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::VIEW, checking, Layout::SPARSE>;

template <class T, Checking checking = _default_checking>
using ConstSparseMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONST_VIEW, checking, Layout::SPARSE>;

// ==================
// --- Formatters ---
// ==================

namespace format {

// --- Implementation ---
// ----------------------

// The "header" of all human-readable formats that displays
// some meta info with tensor type and dimensions.
template <utl_mvl_tensor_arg_defs>
[[nodiscard]] std::string _tensor_meta_string(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor) {
    std::string buffer;

    if constexpr (_type == Type::DENSE) buffer += "Dense";
    if constexpr (_type == Type::STRIDED) buffer += "Strided";
    if constexpr (_type == Type::SPARSE) buffer += "Sparse";
    if constexpr (_dimension == Dimension::VECTOR) buffer += stringify(" vector [size = ", tensor.size(), "]:\n");
    if constexpr (_dimension == Dimension::MATRIX)
        buffer += stringify(" matrix [size = ", tensor.size(), "] (", tensor.rows(), " x ", tensor.cols(), "):\n");

    return buffer;
}

// Human-readable formats automatically collapse matrices
// that are too  large to be reasonably parsed by a human
constexpr std::size_t _max_displayed_flat_size = 30 * 30;

template <utl_mvl_tensor_arg_defs>
[[nodiscard]] std::string _as_too_large(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor) {
    return stringify(_tensor_meta_string(tensor), "  <hidden due to large size>\n");
}

// Generic method to do "dense matrix print" with given delimiters.
// Cuts down on repetition since a lot of formats only differ in the delimiters used.
template <class T, Type type, Ownership ownership, Checking checking, Layout layout, class Func>
[[nodiscard]] std::string
_generic_dense_format(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor,     //
                      std::string_view                                                              begin,      //
                      std::string_view                                                              row_begin,  //
                      std::string_view                                                              col_delim,  //
                      std::string_view                                                              row_end,    //
                      std::string_view                                                              row_delim,  //
                      std::string_view                                                              end,        //
                      Func                                                                          stringifier //
) {
    if (tensor.empty()) return (std::string() += begin) += end;

    Matrix<std::string> strings(tensor.rows(), tensor.cols());

    // Stringify
    if constexpr (type == Type::SPARSE) strings.fill("-");
    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) { strings(i, j) = stringifier(elem); });
    // this takes care of sparsity, if the matrix is sparse we prefill 'strings' with "-" and then fill appropriate
    // {i, j} with actual stringified values from the tensor. For dense matrices no unnecessary work is done.

    // Get column widths - we want matrix to format nice and aligned
    std::vector<std::size_t> column_widths(strings.cols(), 0);
    for (std::size_t i = 0; i < strings.rows(); ++i)
        for (std::size_t j = 0; j < strings.cols(); ++j)
            column_widths[j] = std::max(column_widths[j], strings(i, j).size());

    // Format with proper alignment
    std::string buffer(begin);
    for (std::size_t i = 0; i < strings.rows(); ++i) {
        buffer += row_begin;
        for (std::size_t j = 0; j < strings.cols(); ++j) {
            if (strings(i, j).size() < column_widths[j]) buffer.append(column_widths[j] - strings(i, j).size(), ' ');
            buffer += strings(i, j);
            if (j + 1 < strings.cols()) buffer += col_delim;
        }
        buffer += row_end;
        if (i + 1 < strings.rows()) buffer += row_delim;
    }
    buffer += end;

    return buffer;
}

// --- Human-readable formats ---
// ------------------------------

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_vector(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    std::string buffer = _tensor_meta_string(tensor);

    buffer += "  { ";
    for (std::size_t idx = 0; idx < tensor.size(); ++idx) {
        buffer += stringifier(tensor[idx]);
        if (idx + 1 < tensor.size()) buffer += ", ";
    }
    buffer += " }\n";

    return buffer;
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_dictionary(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor,
                                        Func                                          stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    std::string buffer = _tensor_meta_string(tensor);

    if constexpr (_dimension == Dimension::MATRIX) {
        tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) {
            buffer += stringify("(", i, ", ", j, ") = ");
            buffer += stringifier(elem);
            buffer += '\n';
        });
    } else {
        tensor.for_each([&](const T& elem, std::size_t i) {
            buffer += stringify("(", i, ") = ");
            buffer += stringifier(elem);
            buffer += '\n';
        });
    }

    return buffer;
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_matrix(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    return _generic_dense_format(tensor, _tensor_meta_string(tensor), "  [ ", " ", " ]\n", "", "", stringifier);
}

// --- Export formats ---
// ----------------------

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_raw(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "", "", " ", "\n", "", "", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_csv(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "", "", ", ", "\n", "", "", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_json(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "[\n", "    [ ", ", ", " ]", ",\n", "\n]\n", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_mathematica(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor,
                                         Func                                          stringifier = Func()) {
    return _generic_dense_format(tensor, "{\n", "    { ", ", ", " }", ",\n", "\n}\n", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_latex(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "\\begin{pmatrix}\n", "  ", " & ", " \\\\\n", "", "\\end{pmatrix}\n",
                                 stringifier);
}


} // namespace format

// ================================
// --- Linear algebra operators ---
// ================================

// --- Unary operator implementation ----
// --------------------------------------

template <class L, class Op,                                                    //
          _is_tensor_enable_if<L> = true,                                       //
          class return_type       = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_unary_op(L&& left, Op&& op) {
    using reference = typename std::decay_t<L>::reference;

    // Reuse r-value if possible
    return_type res = std::forward<L>(left);

    // '.for_each()' takes care of possible sparsity
    res.for_each([&](reference elem) { elem = op(std::move(elem)); });

    return res;
}

// --- Unary operator API ----
// ---------------------------

// std <functional> has function objects for every operator possible,
// except unary '+' for some reason, which is why we have to implement it ourselves.
// This is the exact same thing as 'std::negate<>' but for operator '+'.
template <class T>
struct _unary_plus_functor {
    constexpr T operator()(const T& lhs) const { return +lhs; }
};

template <class L, _is_tensor_enable_if<L> = true, class value_type = typename std::decay_t<L>::value_type,
          _has_unary_op_plus_enable_if<value_type> = true>
auto operator+(L&& left) {
    return apply_unary_op(std::forward<L>(left), _unary_plus_functor<value_type>());
}

template <class L, _is_tensor_enable_if<L> = true, class value_type = typename std::decay_t<L>::value_type,
          _has_unary_op_minus_enable_if<value_type> = true>
auto operator-(L&& left) {
    return apply_unary_op(std::forward<L>(left), std::negate<value_type>());
}

// --- Binary operator implementation ---
// --------------------------------------

// Doing things "in a dumb but simple way" would be to just have all operators take arguments as const-refs and
// return a copy, however we can speed things up a lot by properly using perfect forwarding, which would reuse
// r-values if possible to avoid allocation. Doing so would effectively change something like this:
//    res = A + B - C - D + E
// from 5 (!) copies to only 1, since first operator will create an r-value that gets propagated and reused by
// all others. This however introduces it's own set of challenges since instead of traditional overloading we
// now have to resort to SFINAE-driven imitation of it and carefully watch what we restrict.
//
// So here are the key points of implementing all of this:
//
//    1. All unary and binary operators ('-', '+', '*' and etc.) can benefit from reusing r-values and moving
//       one of the arguments into the result rather than copy. This can only happen to CONTAINER args that
//       correspond to the return type.
//
//    2. Binary operators have following return types for different tensors:
//       - (1)  dense +  dense =>  dense      (complexity O(N^2))
//       - (2)  dense + sparse =>  dense      (complexity O(N)  )
//       - (3) sparse +  dense =>  dense      (complexity O(N)  )
//       - (4) sparse + sparse => sparse      (complexity O(N)  )
//       return types inherit option arguments from the lhs and always have 'ownership == CONTAINER',
//       while argument can be anything, including views. To implement the list above efficiently there
//       is no other way but to make 4 separate implementation, tailored for each level of sparsity.
//
//    3. All binary operators can be written in a generic form like 'apply_binary_operator(A, B, op)'
//       and specialized with operator functors froms std <functional>. All we need for individual
//       operators is to figure out correct boilerplate with perfect forwarding. "Overload resolution"
//       will be performed by the 'apply_operator' method.
//
// It's a good question whether to threat binary '*' as element-wise product (which would be in line with other
// operators) or a matrix product, but in the end it seems doing it element-wise would be too confusing for the
// user, so we leave '*' as a matrix product and declare element-wise product as a function 'elementwise_product()'
// since no existing operators seem suitable for such overload.

// (1)  dense +  dense =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                       //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<L>::reference;
    using size_type = typename std::decay_t<L>::size_type;

    return_type res;

    // Reuse r-value arguments if possible while preserving the order of operations
    if constexpr (std::is_rvalue_reference_v<L> && std::decay_t<L>::params::ownership == Ownership::CONTAINER) {
        res = std::forward<L>(left);
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(std::move(elem), right(i, j)); });
    } else if constexpr (std::is_rvalue_reference_v<R> && std::decay_t<R>::params::ownership == Ownership::CONTAINER) {
        res = std::forward<R>(right);
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(left(i, j), std::move(elem)); });
    } else {
        res = return_type(left.rows(), left.cols());
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(left(i, j), right(i, j)); });
    }

    return res;
}

// (2)  dense + sparse =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                       //
          _is_sparse_tensor_enable_if<R>                    = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<L>::reference;
    using size_type = typename std::decay_t<L>::size_type;

    // Reuse r-value if possible
    return_type res = std::forward<L>(left);

    // '.for_each()' to only iterate sparse elements
    right.for_each([&](reference elem, size_type i, size_type j) { res(i, j) = op(std::move(res(i, j)), elem); });

    return res;
}

// (3) sparse +  dense =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_sparse_tensor_enable_if<L>                    = true,                                       //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                       //
          class return_type                                 = typename std::decay_t<R>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<R>::reference;
    using size_type = typename std::decay_t<R>::size_type;

    // Reuse r-value if possible
    return_type res = std::forward<R>(right);

    // '.for_each()' to only iterate sparse elements
    left.for_each([&](reference elem, size_type i, size_type j) { res(i, j) = op(elem, std::move(res(i, j))); });

    return res;
}

// (4) sparse + sparse => sparse
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_sparse_tensor_enable_if<L>                    = true,                                       //
          _is_sparse_tensor_enable_if<R>                    = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using sparse_entry = typename std::decay_t<L>::sparse_entry_type;
    using value_type   = typename std::decay_t<L>::value_type;

    std::vector<sparse_entry> res_triplets;
    res_triplets.reserve(std::max(left.size(), right.size()));
    // not enough when matrices have different sparsity patterns, but good enough for initial guess

    std::size_t i = 0, j = 0;

    // Merge sparsity patterns
    while (i < left.size() && j < right.size()) {
        // Entry present only in lhs sparsity pattern
        if (left._data[i] < right._data[j]) {
            res_triplets.emplace_back(
                _apply_binary_op_to_sparse_entry_and_value(std::forward<L>(left)._data[i++], value_type{}, op));
        }
        // Entry present only in rhs sparsity pattern
        else if (left._data[i] > right._data[j]) {
            res_triplets.emplace_back(
                _apply_binary_op_to_value_and_sparse_entry(value_type{}, std::forward<R>(right)._data[j++], op));
        }
        // Entry present in both sparsity patterns
        else {
            res_triplets.emplace_back(_apply_binary_op_to_sparse_entries(std::forward<L>(left)._data[i++],
                                                                         std::forward<R>(right)._data[j++], op));
        }
    }
    // Copy the rest of lhs (if needed)
    while (i < left.size()) res_triplets.emplace_back(std::forward<L>(left)._data[i++]);
    // Copy the rest of rhs (if needed)
    while (j < right.size()) res_triplets.emplace_back(std::forward<R>(right)._data[j++]);

    return return_type(left.rows(), left.cols(), std::move(res_triplets));
}

// --- Binary operator API ---
// ---------------------------

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_binary_op_plus_enable_if<value_type> = true>
auto operator+(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::plus<value_type>());
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_binary_op_minus_enable_if<value_type> = true>
auto operator-(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::minus<value_type>());
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type                                = typename std::decay_t<L>::value_type,
          _has_binary_op_multiplies_enable_if<value_type> = true>
auto elementwise_product(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::multiplies<value_type>());
}

// --- Augmented assignment operator API ---
// -----------------------------------------

// These can just reuse corresponding binary operators while 'std::move()'ing lhs to avoid copying.

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_assignment_op_plus_enable_if<value_type> = true>
L& operator+=(L&& left, R&& right) {
    return (left = std::move(left) + right);
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type                               = typename std::decay_t<L>::value_type,
          _has_assignment_op_minus_enable_if<value_type> = true>
L& operator-=(L&& left, R&& right) {
    return (left = std::move(left) - right);
}

// --- Matrix multiplication ---
// -----------------------------

// Just like with binary operators, we run into the need to have 4 different implementations:
//    - (1)  dense +  dense =>  dense      (complexity O(N^3))
//    - (2)  dense + sparse =>  dense      (complexity O(N^2))
//    - (3) sparse +  dense =>  dense      (complexity O(N^2))
//    - (4) sparse + sparse => sparse      (complexity O(N)  )
//
// Would be gread to implement proper "smart" GEMM and CRS-multiplication for sparse matrices, however that adds
// an absolutely huge amount of complexity for x1.5-x3 speedup on matrix multiplication, which is already kind of
// an afterthought provided here mainly for convenience and not for actual number crunching usage.
//
// We do however use some basic optimizations like loop reordering / temporaries / blocking to have a sensible baseline,
// we properly account for sparsity which brings multiplication with sparse matrices from O(N^3) down to O(N^2) / O(N).

// (1)  dense +  dense =>  dense
//
// From benchmarks 1D blocking over "k" with a decently large block size seems to be more reliable than 2D/3D blocking.
// When measuring varying matrix sizes and datatypes, for matrices with less than ~1k rows/cols (aka matrices that fully
// fit into L2 cache) 1D blocking doesn't  seem to have much of an effect (positive or negative), while 2D/3D has often
// lead to a slowdown for specific matrix  sizes. At large enough sizes blocking leads to a noticeable (~2x) speedup.
// Note that this is all very hardware-specific, so it's difficult to make a truly generic judgement. In general,
// blocking seems to be worth it.
//
// Note that unlike other binary operators, here there is no possible benefit in r-value reuse.
//
template <class L, class R,                                                                                //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                        //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                        //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                        //
          class value_type                                  = typename std::decay_t<L>::value_type,        //
          class return_type                                 = typename std::decay_t<L>::owning_reflection, //
          _has_binary_op_multiplies_enable_if<value_type>   = true,                                        //
          _has_assignment_op_plus_enable_if<value_type>     = true                                         //
          >
return_type operator*(const L& left, const R& right) {
    utl_mvl_assert(left.cols() == right.rows());

    using size_type = typename std::decay_t<L>::size_type;

    const size_type N_i = left.rows(), N_k = left.cols(), N_j = right.cols();
    // (N_i)x(N_k) * (N_k)x(N_j) => (N_i)x(N_j)

    constexpr size_type block_size_kk = 32;

    return_type res(N_i, N_j, value_type{});

    for (size_type kk = 0; kk < N_k; kk += block_size_kk) {
        const size_type k_extent = std::min(N_k, kk + block_size_kk);
        // needed for matrices that aren't a multiple of block size
        for (size_type i = 0; i < N_i; ++i) {
            for (size_type k = kk; k < k_extent; ++k) {
                const auto& r = left(i, k);
                for (size_type j = 0; j < N_j; ++j) res(i, j) += r * right(k, j);
            }
        }
    }

    return res;
}

// (2)  dense + sparse =>  dense

// TODO:

// (3) sparse +  dense =>  dense

// TODO:

// (4) sparse + sparse => sparse

// TODO:

// Clear out internal macros
#undef utl_mvl_tensor_arg_defs
#undef utl_mvl_tensor_arg_vals
#undef utl_mvl_require
#undef utl_mvl_reqs

} // namespace utl::mvl

#endif
#endif // module utl::mvl






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::parallel
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_parallel.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PARALLEL)
#ifndef UTLHEADERGUARD_PARALLEL
#define UTLHEADERGUARD_PARALLEL

#define UTL_PARALLEL_VERSION_MAJOR 2
#define UTL_PARALLEL_VERSION_MINOR 1
#define UTL_PARALLEL_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <condition_variable> // condition_variable
#include <cstdint>            // uint64_t
#include <functional>         // plus<>, multiplies<>, function<>
#include <future>             // future<>, promise<>
#include <memory>             // shared_ptr<>
#include <mutex>              // mutex, scoped_lock<>, unique_lock<>
#include <optional>           // optional<>, nullopt
#include <queue>              // queue<>, deque<>, size_t, ptrdiff_t
#include <stdexcept>          // current_exception, runtime_error
#include <thread>             // thread
#include <utility>            // forward<>()
#include <vector>             // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Work-stealing summary:
//
//    - We use several queues:
//         - All  threads have a global task queue
//         - Each thread  has  a local  task deque
//    - Tasks go into different queues depending on their source:
//         - Work queued from a     pool thread => recursive task, goes to the front of local  deque
//         - Work queued from a non-pool thread => external  task, goes to the back  of global queue
//    - When threads are looking for work they search in 3 steps:
//         1. Check local  deque,  work here can be popped from the front
//         2. Check other  deques, work here can be stolen from the back
//         3. Check global queue,  work here can be popped from the front
//    - To resolve recursive deadlocks we use a custom future:
//         - Recursive task calls '.wait()' on its future => pop / steal work from local deques until finished
//
// Ideally we would want to use a different algorithm with Chase-Lev lock-free local SPMC queues for work-stealing,
// and a global lock-free MPMC queue for outside tasks, however properly implementing such queues is a task of
// incredible complexity, which is further exacerbated by the fact that 'std::function' has a potentially throwing
// move-assignment, which disqualifies it from ~80% of existing lock-free queue implementations.
//
// Newer standards would enable several improvements in terms of implementation:
//    - In C++20 'std::jthread' can be used to simplify joining and add stop tokens
//    - In C++23 'std::move_only_function<>' can be used as a more efficient task type,
//      however this still doesn't resolve the issue of throwing move assignment
//    - In C++20 atomic wait / semaphores / barriers can be used to improve efficiency
//      of some syncronization
//
// It is also possible to add task priority with a bit of constexpr logic and possibly reduce locking
// in some parts of the scheduling, this is a work for future releases.

// ____________________ IMPLEMENTATION ____________________

namespace utl::parallel::impl {

// ============================
// --- Thread introspection ---
// ============================

namespace this_thread {

inline thread_local std::optional<std::size_t> worker_index    = std::nullopt;
inline thread_local std::optional<void*>       thread_pool_ptr = std::nullopt;

[[nodiscard]] inline std::optional<std::size_t> get_index() noexcept { return worker_index; }
[[nodiscard]] inline std::optional<void*>       get_pool() noexcept { return thread_pool_ptr; }

}; // namespace this_thread

[[nodiscard]] inline std::size_t hardware_concurrency() noexcept {
    const std::size_t     detected_count = std::thread::hardware_concurrency();
    constexpr std::size_t fallback_count = 4;
    return detected_count ? detected_count : fallback_count;
    // if 'hardware_concurrency()' struggles to determine the number of threads, we fallback onto a reasonable default
}

// ===================
// --- Thread pool ---
// ===================

class ThreadPool;

namespace ws_this_thread { // same this as thread introspection from public API, but more convenient for internal use
inline thread_local ThreadPool* thread_pool_ptr = nullptr;
inline thread_local std::size_t worker_index    = std::size_t(-1);
}; // namespace ws_this_thread

inline std::size_t splitmix64() noexcept {
    thread_local std::uint64_t state = ws_this_thread::worker_index;

    std::uint64_t result = (state += 0x9E3779B97f4A7C15);
    result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return static_cast<std::size_t>(result ^ (result >> 31));
} // very fast & simple PRNG

class ThreadPool {
    using task_type         = std::function<void()>;
    using global_queue_type = std::queue<task_type>;
    using local_queue_type  = std::deque<task_type>;

    std::vector<std::thread> workers;
    std::mutex               workers_mutex;

    global_queue_type global_queue;
    std::mutex        global_queue_mutex;

    std::vector<local_queue_type> local_queues;
    std::vector<std::mutex>       local_queues_mutexes;

    std::condition_variable task_available_cv;
    std::condition_variable task_done_cv;
    std::mutex              task_mutex;

    std::size_t tasks_running = 0; // protected by task mutex
    std::size_t tasks_pending = 0;

    bool waiting     = false; // protected by task mutex
    bool terminating = true;

private:
    void spawn_workers(std::size_t count) {
        this->workers              = std::vector<std::thread>(count);
        this->local_queues         = std::vector<local_queue_type>(count);
        this->local_queues_mutexes = std::vector<std::mutex>(count);
        {
            const std::scoped_lock task_lock(this->task_mutex);
            this->terminating = false;
        }
        for (std::size_t i = 0; i < count; ++i) this->workers[i] = std::thread([this, i] { this->worker_main(i); });
    }

    void terminate_workers() {
        {
            const std::scoped_lock polling_lock(this->task_mutex);
            this->terminating = true;
        }
        this->task_available_cv.notify_all();

        for (std::size_t i = 0; i < this->workers.size(); ++i)
            if (this->workers[i].joinable()) this->workers[i].join();
    }

    void worker_main(std::size_t worker_index) {
        this_thread::thread_pool_ptr    = this;
        this_thread::worker_index       = worker_index;
        ws_this_thread::thread_pool_ptr = this;
        ws_this_thread::worker_index    = worker_index;

        while (true) {
            std::unique_lock task_lock(this->task_mutex);

            // Wake up thread pool 'wait()' if necessary
            if (this->waiting && !this->tasks_pending && !this->tasks_running) this->task_done_cv.notify_all();

            // Tasks pending       => continue execution and pull tasks to execute
            // Pool is terminating => continue execution and break out of the main loop
            // otherwise           => wait
            this->task_available_cv.wait(task_lock, [this] { return this->terminating || this->tasks_pending; });

            // Terminate if necessary
            if (this->terminating) break;

            task_lock.unlock();

            // Try to pull in a task
            task_type task;
            if (this->try_pop_local(task) || this->try_steal(task) || this->try_pop_global(task)) {
                task_lock.lock();
                --this->tasks_pending;
                ++this->tasks_running;
                task_lock.unlock();

                task();

                task_lock.lock();
                --this->tasks_running;
                task_lock.unlock();
            }
        }

        this_thread::thread_pool_ptr    = std::nullopt;
        this_thread::worker_index       = std::nullopt;
        ws_this_thread::thread_pool_ptr = nullptr;
        ws_this_thread::worker_index    = std::size_t(-1);
    }

    bool try_pop_local(task_type& task) {
        const std::scoped_lock local_queue_lock(this->local_queues_mutexes[ws_this_thread::worker_index]);

        auto& local_queue = this->local_queues[ws_this_thread::worker_index];

        if (local_queue.empty()) return false;

        task = std::move(local_queue.front());
        local_queue.pop_front();
        return true;
    }

    bool try_steal(task_type& task) {
        for (std::size_t attempt = 0; attempt < this->workers.size(); ++attempt) {
            const std::size_t i = splitmix64() % this->workers.size();

            if (i == ws_this_thread::worker_index) continue; // don't steal from yourself

            const std::scoped_lock local_queue_lock(this->local_queues_mutexes[i]);

            auto& local_queue = this->local_queues[i];

            if (local_queue.empty()) continue;

            task = std::move(local_queue.back());
            local_queue.pop_back();
            return true;
        }

        return false;
    }

    bool try_pop_global(task_type& task) {
        const std::scoped_lock global_queue_lock(this->global_queue_mutex);

        if (this->global_queue.empty()) return false;

        task = std::move(this->global_queue.front());
        this->global_queue.pop();
        return true;
    }

public:
    explicit ThreadPool(std::size_t count = std::thread::hardware_concurrency()) { this->spawn_workers(count); }

    ~ThreadPool() noexcept {
        try {
            this->wait();
            this->terminate_workers();
        } catch (...) {} // no throwing from the destructor
    }

    template <class T = void>
    struct future_type {
        std::future<T> future;

        void fallthrough() const {
            ThreadPool* pool = ws_this_thread::thread_pool_ptr;
            
            if (!pool) return;
            
            // Execute recursive tasks from local queues until this future is ready
            task_type task;
            while (pool->try_pop_local(task) || pool->try_steal(task)) {
                pool->task_mutex.lock();
                --pool->tasks_pending;
                ++pool->tasks_running;
                pool->task_mutex.unlock();
            
                task();
            
                pool->task_mutex.lock();
                --pool->tasks_running;
                pool->task_mutex.unlock();
                
                if (this->is_ready()) return;
            }
        }
        
    bool is_ready() const {
        return this->future.wait_for(std::chrono::seconds{0}) == std::future_status::ready;
    }

    public:
        future_type(std::future<T>&& future) : future(std::move(future)) {} // conversion from regular future
        future_type& operator=(std::future<T>&& future) {
            this->future = std::move(future);
            return *this;
        }

        using is_recursive = void;

        auto get() {
            this->fallthrough();
            return this->future.get();
        }

        bool valid() const noexcept { return this->future.valid(); }

        void wait() const {
            this->fallthrough();
            this->future.wait();
        }

        template <class Rep, class Period>
        std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
            this->fallthrough();
            return this->future.wait_for(timeout_duration);
        }

        template <class Clock, class Duration>
        std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) const {
            this->fallthrough();
            return this->future.wait_until(timeout_time);
        }
    };

    void set_thread_count(std::size_t count) {
        if (ws_this_thread::thread_pool_ptr == this)
            throw std::runtime_error("Cannot resize thread pool from its own pool thread.");

        const std::scoped_lock workers_lock(this->workers_mutex);

        this->wait();
        this->terminate_workers();
        this->spawn_workers(count);
    }

    [[nodiscard]] std::size_t get_thread_count() {
        if (ws_this_thread::thread_pool_ptr == this) return this->workers.size();
        // calls from inside the pool shouldn't lock, otherwise we could deadlock the thread by trying to
        // query the thread count while the pool was instructed to resize, worker count is constant during
        // a worker lifetime so it's safe to query without a lock anyways

        const std::scoped_lock workers_lock(this->workers_mutex);

        return this->workers.size();
    }

    void wait() {
        std::unique_lock task_lock(this->task_mutex);

        this->waiting = true;
        this->task_done_cv.wait(task_lock, [this] { return !this->tasks_pending && !this->tasks_running; });
        this->waiting = false;
    }

    template <class F, class... Args>
    void detached_task(F&& f, Args&&... args) {
        auto closure = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // Recursive task
        if (ws_this_thread::thread_pool_ptr == this) {
            const std::scoped_lock local_queue_lock(this->local_queues_mutexes[ws_this_thread::worker_index]);
            this->local_queues[ws_this_thread::worker_index].push_front(std::move(closure));
        }
        // Regular task
        else {
            const std::scoped_lock global_queue_mutex(this->global_queue_mutex);
            this->global_queue.emplace(std::move(closure));
        }

        {
            const std::scoped_lock task_lock(this->task_mutex);
            ++this->tasks_pending;
        }
        this->task_available_cv.notify_one();
    }

    template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    future_type<R> awaitable_task(F&& f, Args&&... args) {
        auto closure = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        const std::shared_ptr<std::promise<R>> promise = std::make_shared<std::promise<R>>();
        // ideally we could just move promise into a lambda and use it as is, however that makes the lambda
        // non-copyable (since promise itself is move-only) which doesn't work with 'std::function<>', this
        // is fixed in C++23 with 'std::move_only_function<>'

        future_type<R> future = promise->get_future();

        this->detached_task([closure = std::move(closure), promise = std::move(promise)]() mutable {
            try {
                if constexpr (std::is_void_v<R>) {
                    closure();
                    promise->set_value();
                } else {
                    promise->set_value(closure());
                } // 'promise->set_value(f())' when 'f()' returns 'void' is a compile error, so we use a workaround
            } catch (...) {
                try {
                    promise->set_exception(std::current_exception()); // this may still throw
                } catch (...) {}
            }
        });

        return future;
    }
};

template <class T = void>
using Future = ThreadPool::future_type<T>;

// ==============
// --- Ranges ---
// ==============

// --- SFINAE helpers ---
// ----------------------

template <bool Cond>
using require = std::enable_if_t<Cond, bool>;

template <class T, class... Args>
using require_invocable = require<std::is_invocable_v<T, Args...>>;
// this is simple convenience

template <class T, class = void>
struct has_iter : std::false_type {};
template <class T>
struct has_iter<T, std::void_t<decltype(std::declval<typename T::iterator>())>> : std::true_type {};

template <class T, class = void>
struct has_const_iter : std::false_type {};
template <class T>
struct has_const_iter<T, std::void_t<decltype(std::declval<typename T::const_iterator>())>> : std::true_type {};

template <class T>
using require_has_some_iter = require<has_iter<T>::value || has_const_iter<T>::value>;
// needed to restrict template 'Range' constructor from a container, otherwise it takes priority over copy / move ctors

template <class T, class = void>
struct is_recursive : std::false_type {};
template <class T>
struct is_recursive<T, std::void_t<decltype(std::declval<typename T::is_recursive>())>> : std::true_type {};
template <class T>
constexpr bool is_recursive_v = is_recursive<T>::value;

// --- Utils ---
// -------------

[[nodiscard]] constexpr std::size_t min_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? b : a; }
[[nodiscard]] constexpr std::size_t max_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? a : b; }
// no reason to include the entirety of <algorithm> just for 2 one-liners

constexpr std::size_t default_grains_per_thread = 4;
// by default we distribute 4 tasks per thread, this number is purely empirical. We don't want to split up
// work into too many tasks (like with 'grain_size = 1'), yet we want it to be a bit more granular than
// doing 1 task per thread since that would be horrible if tasks are noticeably uneven.

// --- Range ---
// -------------

template <class It>
struct Range {
    It          begin;
    It          end;
    std::size_t grain_size;

    Range() = delete;

    constexpr Range(It begin, It end, std::size_t grain_size) : begin(begin), end(end), grain_size(grain_size) {}

    Range(It begin, It end)
        : Range(begin, end, max_size(1, (end - begin) / (hardware_concurrency() * default_grains_per_thread))) {}


    template <class Container, require<has_const_iter<Container>::value> = true>
    Range(const Container& container) : Range(container.begin(), container.end()) {}

    template <class Container, require<has_iter<Container>::value> = true>
    Range(Container& container) : Range(container.begin(), container.end()) {}
}; // requires random-access iterator, but no good way to express that before C++20 concepts

// CTAD for deducing iterator range from a container
template <class Container>
Range(const Container& container) -> Range<typename Container::const_iterator>;

template <class Container>
Range(Container& container) -> Range<typename Container::iterator>;

// --- Index range ---
// -------------------

template <class Idx = std::ptrdiff_t>
struct IndexRange {
    Idx         first;
    Idx         last;
    std::size_t grain_size;

    IndexRange() = delete;

    constexpr IndexRange(Idx first, Idx last, std::size_t grain_size)
        : first(first), last(last), grain_size(grain_size) {}

    IndexRange(Idx first, Idx last)
        : IndexRange(first, last, max_size(1, (last - first) / (hardware_concurrency() * default_grains_per_thread))) {}

    template <class Idx1, class Idx2>
    constexpr IndexRange(Idx1 first, Idx2 last, std::size_t grain_size)
        : first(first), last(last), grain_size(grain_size) {}

    template <class Idx1, class Idx2>
    IndexRange(Idx1 first, Idx2 last)
        : IndexRange(first, last, max_size(1, (last - first) / (hardware_concurrency() * default_grains_per_thread))) {}
};

// Note: It is common to have a ranges from 'int' to 'std::size_t' (for example 'IndexRange{0, vec.size()}'),
//       in such cases we assume 'std::ptrdiff_t' as a reasonable default

// =================
// --- Scheduler ---
// =================

// There is a ton of boilerplate since we need a whole bunch of convenience overloads
// (ranges / index ranges / containers, blocked functions / iteration functions, etc.),
// but conceptually it's all quite simple we end up with 2 overloads for tasks,
// 6+6+3=15 overloads for parallel-for and 4+4+2=10 overloads for reduce

#define utl_parallel_assert_message                                                                                    \
    "Awaitable loops require recursive task support from the 'future_type' of 'Scheduler' backend. "                   \
    "Future can signal such support by  providing a 'using is_recursive = void' member typedef."
// 'static_assert()' only supports string literals, cannot use constexpr variable here, assert itself
// shouldn't be included in the macro as it makes error messages uglier due to macro expansion

template <class Backend = ThreadPool>
struct Scheduler {

    // --- Backend ---
    // ---------------

    Backend backend; // underlying thread pool

    template <class T = void>
    using future_type = typename Backend::template future_type<T>;

    template <class... Args>
    explicit Scheduler(Args&&... args) : backend(std::forward<Args>(args)...) {}

    // --- Task API ---
    // ----------------

    template <class F, class... Args>
    void detached_task(F&& f, Args&&... args) {
        this->backend.detached_task(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    future_type<R> awaitable_task(F&& f, Args&&... args) {
        return this->backend.awaitable_task(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // --- Parallel-for API ---
    // ------------------------

    // - 'Range' overloads (6) -

    template <class It, class F, require_invocable<F, It, It> = true> // blocked loop iteration overload
    void detached_loop(Range<It> range, F&& f) {
        for (It it = range.begin; it < range.end; it += min_size(range.grain_size, range.end - it))
            this->detached_task(f, it, it + min_size(range.grain_size, range.end - it));
        // 'min_size(...)' bit takes care of the unevenly sized tail segment
    }

    template <class It, class F, require_invocable<F, It> = true> // single loop iteration overload
    void detached_loop(Range<It> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](It low, It high) { // combine individual index
            for (It it = low; it < high; ++it) f(it);                    // calls into blocks and forward
        };                                                               // into a blocked loop iteration overload
        this->detached_loop(range, std::move(iterate_block));
    }

    template <class It, class F, require_invocable<F, It, It> = true>
    void blocking_loop(Range<It> range, F&& f) {
        std::vector<future_type<>> futures;

        for (It it = range.begin; it < range.end; it += min_size(range.grain_size, range.end - it))
            futures.emplace_back(this->awaitable_task(f, it, it + min_size(range.grain_size, range.end - it)));

        for (auto& future : futures) future.wait();
    }

    template <class It, class F, require_invocable<F, It> = true>
    void blocking_loop(Range<It> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](It low, It high) {
            for (It it = low; it < high; ++it) f(it);
        };
        this->blocking_loop(range, std::move(iterate_block));
    }

    template <class It, class F, require_invocable<F, It, It> = true>
    future_type<> awaitable_loop(Range<It> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto submit_loop = [this, range, f = std::forward<F>(f)] { this->blocking_loop(range, f); };
        return this->awaitable_task(std::move(submit_loop));
        // to wait for multiple tasks we need a vector of futures, since logically we want API to return regular
        // 'future_type<>' rather than some composite construct, we can wrap block awaiting into another task
        // that will "merge" all those futures into a single one for the user, this requires recursive task
        // support from the backend so it's provided conditionally
    }

    template <class It, class F, require_invocable<F, It> = true>
    future_type<> awaitable_loop(Range<It> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto iterate_block = [f = std::forward<F>(f)](It low, It high) {
            for (It it = low; it < high; ++it) f(it);
        };
        return this->awaitable_loop(range, std::move(iterate_block));
    }

    // - 'IndexRange' overloads (6) -

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    void detached_loop(IndexRange<Idx> range, F&& f) {
        for (Idx i = range.first; i < range.last; i += static_cast<Idx>(range.grain_size))
            this->detached_task(f, i, static_cast<Idx>(min_size(i + range.grain_size, range.last)));
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    void detached_loop(IndexRange<Idx> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        this->detached_loop(range, std::move(iterate_block));
    }

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    void blocking_loop(IndexRange<Idx> range, F&& f) {
        std::vector<future_type<>> futures;

        for (Idx i = range.first; i < range.last; i += static_cast<Idx>(range.grain_size))
            futures.emplace_back(
                this->awaitable_task(f, i, static_cast<Idx>(min_size(i + range.grain_size, range.last))));

        for (auto& future : futures) future.wait();
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    void blocking_loop(IndexRange<Idx> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        this->blocking_loop(range, std::move(iterate_block));
    }

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    future_type<> awaitable_loop(IndexRange<Idx> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto submit_loop = [this, range, f = std::forward<F>(f)] { this->blocking_loop(range, f); };
        return this->awaitable_task(std::move(submit_loop));
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    future_type<> awaitable_loop(IndexRange<Idx> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        return this->awaitable_loop(range, std::move(iterate_block));
    }

    // - 'Container' overloads (3) -

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true> // without SFINAE reqs
    void detached_loop(Container&& container, F&& f) {                                         // such overloads would
        this->detached_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));    // always get picked
    }                                                                                          // over the others

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
    void blocking_loop(Container&& container, F&& f) {
        this->blocking_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));
    }

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
    future_type<> awaitable_loop(Container&& container, F&& f) {
        return this->awaitable_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));
    }

    // --- Parallel-reduce API ---
    // ---------------------------

    // - 'Range' overloads (2) -

    template <class It, class Op, class R = typename It::value_type>
    R blocking_reduce(Range<It> range, Op&& op) {
        if (range.begin == range.end) throw std::runtime_error("Reduction over an empty range is undefined");

        R          result = *range.begin;
        std::mutex result_mutex;

        this->blocking_loop(Range<It>{range.begin + 1, range.end}, [&](It low, It high) {
            R partial_result = *low;
            for (auto it = low + 1; it != high; ++it) partial_result = op(partial_result, *it);

            const std::scoped_lock result_lock(result_mutex);
            result = op(result, partial_result);
        });

        return result;
    }

    template <class It, class Op, class R = typename It::value_type>
    future_type<R> awaitable_reduce(Range<It> range, Op&& op) {
        auto submit_reduce = [this, range, op = std::forward<Op>(op)] { return this->blocking_reduce(range, op); };
        return this->awaitable_task(std::move(submit_reduce));
    }

    // - 'Container' overloads (2) -

    template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
    R blocking_reduce(Container&& container, Op&& op) {
        return this->blocking_reduce(Range{std::forward<Container>(container)}, std::forward<Op>(op));
    }

    template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
    future_type<R> awaitable_reduce(Container&& container, Op&& op) {
        return this->awaitable_reduce(Range{std::forward<Container>(container)}, std::forward<Op>(op));
    }
};

#undef utl_parallel_assert_message

// =========================
// --- Binary operations ---
// =========================

// Note 1:
// Binary operations should be implemented as functors similarly to 'std::plus<>',
// defining them as free-standing functions poses a huge challenge for inlining due
// to function pointer indirection, this can cause x2-x4 performance degradation.

// Note 2:
// Sum / prod can be defined as aliases for standard functors. Min / max require custom implementation.
// Note the '<void>' specialization for transparent functors, see "transparent function objects" on
// https://en.cppreference.com/w/cpp/functional.html

template <class T = void>
using sum = std::plus<T>;

template <class T = void>
using prod = std::multiplies<T>;

template <class T = void>
struct min {
    constexpr const T& operator()(const T& lhs, const T& rhs) const noexcept(noexcept((lhs < rhs) ? lhs : rhs)) {
        return (lhs < rhs) ? lhs : rhs;
    }
};

template <>
struct min<void> {
    template <class T1, class T2>
    constexpr auto operator()(T1&& lhs, T2&& rhs) const
        noexcept(noexcept(std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs)))
            -> decltype(std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs)) {
        return std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs);
    }

    using is_transparent = std::less<>::is_transparent;
};

template <class T = void>
struct max {
    constexpr const T& operator()(const T& lhs, const T& rhs) const noexcept(noexcept((lhs < rhs) ? rhs : lhs)) {
        return (lhs < rhs) ? rhs : lhs;
    }
};

template <>
struct max<void> {
    template <class T1, class T2>
    constexpr auto operator()(T1&& lhs, T2&& rhs) const
        noexcept(noexcept(std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs)))
            -> decltype(std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs)) {
        return std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs);
    }

    using is_transparent = std::less<>::is_transparent;
};

// =======================
// --- Global executor ---
// =======================

// A convenient copy of the threadpool & scheduler API hooked up to a global lazily-initialized thread pool

auto& global_scheduler() {
    static Scheduler scheduler;
    return scheduler;
}

// --- Thread pool API ---
// -----------------------

void set_thread_count(std::size_t count = hardware_concurrency()) {
    global_scheduler().backend.set_thread_count(count);
}

std::size_t get_thread_count() { return global_scheduler().backend.get_thread_count(); }

void wait() { global_scheduler().backend.wait(); }

// --- Scheduler API ---
// ---------------------

// Note: We could significantly reduce boilerplate by just using a macro to define functions forwarding everything to
//       global scheduler and returning auto, however this messes up LSP autocomplete for users, so we do it manually

// - Task API -

template <class F, class... Args>
void detached_task(F&& f, Args&&... args) {
    global_scheduler().detached_task(std::forward<F>(f), std::forward<Args>(args)...);
}

template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
Future<R> awaitable_task(F&& f, Args&&... args) {
    return global_scheduler().awaitable_task(std::forward<F>(f), std::forward<Args>(args)...);
}

// - Parallel-for API -

template <class It, class F>
void detached_loop(Range<It> range, F&& f) {
    global_scheduler().detached_loop(range, std::forward<F>(f));
}

template <class It, class F>
void blocking_loop(Range<It> range, F&& f) {
    global_scheduler().blocking_loop(range, std::forward<F>(f));
}

template <class It, class F>
auto awaitable_loop(Range<It> range, F&& f) {
    return global_scheduler().awaitable_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
void detached_loop(IndexRange<Idx> range, F&& f) {
    global_scheduler().detached_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
void blocking_loop(IndexRange<Idx> range, F&& f) {
    global_scheduler().blocking_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
Future<> awaitable_loop(IndexRange<Idx> range, F&& f) {
    return global_scheduler().awaitable_loop(range, std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
void detached_loop(Container&& container, F&& f) {
    global_scheduler().detached_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
void blocking_loop(Container&& container, F&& f) {
    global_scheduler().blocking_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
Future<> awaitable_loop(Container&& container, F&& f) {
    return global_scheduler().awaitable_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class It, class Op, class R = typename It::value_type>
R blocking_reduce(Range<It> range, Op&& op) {
    return global_scheduler().blocking_reduce(range, std::forward<Op>(op));
}

template <class It, class Op, class R = typename It::value_type>
Future<R> awaitable_reduce(Range<It> range, Op&& op) {
    return global_scheduler().awaitable_reduce(range, std::forward<Op>(op));
}

template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
R blocking_reduce(Container&& container, Op&& op) {
    return global_scheduler().blocking_reduce(std::forward<Container>(container), std::forward<Op>(op));
}

template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
Future<R> awaitable_reduce(Container&& container, Op&& op) {
    return global_scheduler().awaitable_reduce(std::forward<Container>(container), std::forward<Op>(op));
}

} // namespace utl::parallel::impl

// ______________________ PUBLIC API ______________________

namespace utl::parallel {

using impl::Scheduler;
using impl::ThreadPool;
using impl::Future;

using impl::Range;
using impl::IndexRange;

using impl::sum;
using impl::prod;
using impl::min;
using impl::max;

using impl::set_thread_count;
using impl::get_thread_count;
using impl::wait;

using impl::detached_task;
using impl::awaitable_task;

using impl::detached_loop;
using impl::blocking_loop;
using impl::awaitable_loop;

using impl::blocking_reduce;
using impl::awaitable_reduce;

namespace this_thread = impl::this_thread;

using impl::hardware_concurrency;

} // namespace utl::parallel

#endif
#endif // module utl::parallel






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

#define UTL_PREDEF_VERSION_MAJOR 2
#define UTL_PREDEF_VERSION_MINOR 0
#define UTL_PREDEF_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <cassert>     // assert()
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
#if defined(_MSC_VER)
#define UTL_PREDEF_FORCE_INLINE __forceinline inline
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define UTL_PREDEF_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define UTL_PREDEF_FORCE_INLINE inline
#endif

// Force noinline
#if defined(_MSC_VER)
#define UTL_PREDEF_NO_INLINE __declspec((noinline))
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define UTL_PREDEF_NO_INLINE __attribute__((noinline))
#endif

// Assume condition
// Note: 'assert()' ensures the assumption actually holds true in Debug
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
#define UTL_PREDEF_ASSUME(...) [[assume(__VA_ARGS__))]];                                                               \
    assert(__VA_ARGS__)
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
#define UTL_PREDEF_ASSUME(...)                                                                                         \
    __assume(__VA_ARGS__);                                                                                             \
    assert(__VA_ARGS__)
#elif defined(UTL_PREDEF_COMPILER_IS_CLANG)
#define UTL_PREDEF_ASSUME(...)                                                                                         \
    __builtin_assume(__VA_ARGS__);                                                                                     \
    assert(__VA_ARGS__)
#else // no equivalent GCC built-in
#define UTL_PREDEF_ASSUME(...) assert(__VA_ARGS__)
#endif

[[noreturn]] inline void unreachable() {
#if defined(UTL_PREDEF_STANDARD_IS_23_PLUS)
    std::unreachable();
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
    __assume(false);
#elif defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
    __builtin_unreachable();
#else
    // even if no extension is used, undefined behavior is still raised
    // by an empty function body and the '[[noreturn]]' attribute
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
    buffer += "L1 cache line (D): ";
    buffer += std::to_string(std::hardware_destructive_interference_size);
    buffer += '\n';

    buffer += "L1 cache line (C): ";
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
// macro -> UTL_PREDEF_NO_INLINE
// macro -> UTL_PREDEF_ASSUME
using impl::unreachable;

using impl::compilation_summary;
// macro -> UTL_PREDEF_VA_ARGS_COUNT
// macro -> UTL_PREDEF_IS_FUNCTION_DEFINED_TRAIT

} // namespace utl::predef

#endif
#endif // module utl::predef






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::profiler
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_profiler.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PROFILER)
#ifndef UTLHEADERGUARD_PROFILER
#define UTLHEADERGUARD_PROFILER

#define UTL_PROFILER_VERSION_MAJOR 1
#define UTL_PROFILER_VERSION_MINOR 0
#define UTL_PROFILER_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#ifndef UTL_PROFILER_DISABLE

#include <array>         // array<>, size_t
#include <cassert>       // assert()
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock, duration<>
#include <cstdint>       // uint16_t, uint32_t
#include <iostream>      // cout
#include <mutex>         // mutex, lock_guard
#include <string>        // string, to_string()
#include <string_view>   // string_view
#include <thread>        // thread::id, this_thread::get_id()
#include <type_traits>   // enable_if_t<>, is_enum_v<>, is_invokable_v<>, underlying_type_t<>
#include <unordered_map> // unordered_map<>
#include <vector>        // vector<>

#endif // no need to pull all these headers with profiling disabled

// ____________________ DEVELOPER DOCS ____________________

// Optional macros:
// - #define UTL_PROFILER_DISABLE                            // disable all profiling
// - #define UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY 3.3e9 // use low-overhead rdtsc timestamps
// - #define UTL_PROFILER_USE_SMALL_IDS                      // use 16-bit ids
//
// This used to be a much simpler header with a few macros to profile scope & print a flat table, it
// already applied the idea of using static variables to mark callsites efficiently and later underwent
// a full rewrite to add proper threading & call graph support.
//
// A lot of though went into making it fast, the key idea is to use 'thread_local' callsite
// markers to associate callsites with numeric thread-specific IDs and to reduce all call graph
// traversal to simple integer array lookups. Store everything we can densely, minimize locks,
// delay formatting and result evaluation as much as possible.
//
// Docs & comments scattered through code should explain the details decently well.

// ____________________ IMPLEMENTATION ____________________

#ifndef UTL_PROFILER_DISABLE
// '#ifndef' that wraps almost entire header,
// in '#else' branch only no-op mocks of the public API are compiled

// ==================================
// --- Optional __rdtsc() support ---
// ==================================

#ifdef UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define utl_profiler_cpu_counter __rdtsc()

#endif

// ====================
// --- String utils ---
// ====================

namespace utl::profiler::impl {

constexpr std::size_t max(std::size_t a, std::size_t b) noexcept {
    return (a < b) ? b : a;
} // saves us a heavy <algorithm> include

template <class... Args>
void append_fold(std::string& str, const Args&... args) {
    ((str += args), ...);
} // faster than 'std::ostringstream' and saves us an include

inline std::string format_number(double value, std::chars_format format, int precision) {
    std::array<char, 30> buffer; // 80-bit 'long double' fits in 29, 64-bit 'double' in 24, this is always enough
    const auto end_ptr = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, format, precision).ptr;
    return std::string(buffer.data(), end_ptr);
}

inline std::string format_call_site(std::string_view file, int line, std::string_view func) {
    const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

    std::string res;
    res.reserve(filename.size() + func.size() + 10); // +10 accounts for formatting chars and up to 5 line digits
    append_fold(res, filename, ":", std::to_string(line), ", ", func, "()");
    return res;
}

inline void append_aligned_right(std::string& str, const std::string& source, std::size_t width, char fill = ' ') {
    assert(width >= source.size());

    const std::size_t pad_left = width - source.size();
    str.append(pad_left, fill) += source;
}

inline void append_aligned_left(std::string& str, const std::string& source, std::size_t width, char fill = ' ') {
    assert(width >= source.size());

    const std::size_t pad_right = width - source.size();
    (str += source).append(pad_right, fill);
}

// ==============
// --- Timing ---
// ==============

// If we know CPU frequency at compile time we can wrap '__rdtsc()' into a <chrono>-compatible
// clock and use it seamlessly, no need for conditional compilation anywhere else

#ifdef UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY
struct clock {
    using rep                   = unsigned long long int;
    using period                = std::ratio<1, static_cast<rep>(UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY)>;
    using duration              = std::chrono::duration<rep, period>;
    using time_point            = std::chrono::time_point<clock>;
    static const bool is_steady = true;

    static time_point now() noexcept { return time_point(duration(utl_profiler_cpu_counter)); }
};
#else
using clock   = std::chrono::steady_clock;
#endif

using duration   = clock::duration;
using time_point = clock::time_point;

using ms = std::chrono::duration<double, std::chrono::milliseconds::period>;
// float time makes conversions more convenient

// =====================
// --- Type-safe IDs ---
// =====================

template <class Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
[[nodiscard]] constexpr auto to_int(Enum value) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(value);
}

#ifdef UTL_PROFILER_USE_SMALL_IDS
using id_type = std::uint16_t;
#else
using id_type = std::uint32_t;
#endif

enum class CallsiteId : id_type { empty = to_int(CallsiteId(-1)) };
enum class NodeId : id_type { root = 0, empty = to_int(NodeId(-1)) };

struct CallsiteInfo {
    const char* file;
    const char* func;
    const char* label;
    int         line;
    // 'file', 'func', 'label' are guaranteed to be string literals, since we want to
    // have as little overhead as possible during runtime, we can just save raw pointers
    // and convert them to nicer types like 'std::string_view' later in the formatting stage
};

// ==================
// --- Formatting ---
// ==================

struct Style {
    std::size_t indent = 2;
    bool        color  = true;

    double cutoff_red    = 0.40; // > 40% of total runtime
    double cutoff_yellow = 0.20; // > 20% of total runtime
    double cutoff_gray   = 0.01; // <  1% of total runtime
};

namespace color {

constexpr std::string_view red          = "\033[31m";   // call graph rows that take very significant time
constexpr std::string_view yellow       = "\033[33m";   // call graph rows that take significant time
constexpr std::string_view gray         = "\033[90m";   // call graph rows that take very little time
constexpr std::string_view bold_cyan    = "\033[36;1m"; // call graph headings
constexpr std::string_view bold_green   = "\033[32;1m"; // joined  threads
constexpr std::string_view bold_magenta = "\033[35;1m"; // running threads
constexpr std::string_view bold_blue    = "\033[34;1m"; // thread runtime

constexpr std::string_view reset = "\033[0m";

} // namespace color

struct FormattedRow {
    CallsiteInfo callsite;
    duration     time;
    std::size_t  depth;
    double       percentage;
};

// =================================
// --- Call graph core structure ---
// =================================

class NodeMatrix {
    template <class T>
    using array_type = std::vector<T>;
    // Note: Using 'std::unique_ptr<T[]> arrays would shave off 64 bytes from 'sizeof(NodeMatrix)',
    //       but it's cumbersome and not particularly important for performance

    constexpr static std::size_t col_growth_mul = 2;
    constexpr static std::size_t row_growth_add = 4;
    // - rows capacity grows additively in fixed increments
    // - cols capacity grows multiplicatively
    // this unusual growth strategy is due to our anticipated growth pattern - callsites are few, every
    // one needs to be manually created by the user, nodes can quickly grow in number due to recursion

    array_type<NodeId> prev_ids;
    // [ nodes ] dense vector encoding backwards-traversal of a call graph
    // 'prev_ids[node_id]' -> id of the previous node in the call graph for 'node_id'

    array_type<NodeId> next_ids;
    // [ callsites x nodes ] dense matrix encoding forward-traversal of a call graph
    // 'next_ids(callsite_id, node_id)' -> id of the next node in the call graph for 'node_id' at 'callsite_id',
    //                                     storage is col-major due to our access pattern

    // Note: Both 'prev_ids' and 'next_ids' will contain 'NodeId::empty' values at positions with no link

    array_type<duration> times;
    // [ nodes ] dense vector containing time spent at each node of the call graph
    // 'times[node_id]' -> total time spent at 'node_id'

    array_type<CallsiteInfo> callsites;
    // [ callsites ] dense vector containing info about the callsites
    // 'callsites[callsite_id]' -> pointers to file/function/label & line

    std::size_t rows_size     = 0;
    std::size_t cols_size     = 0;
    std::size_t rows_capacity = 0;
    std::size_t cols_capacity = 0;

public:
    std::size_t rows() const noexcept { return this->rows_size; }
    std::size_t cols() const noexcept { return this->cols_size; }

    bool empty() const noexcept { return this->rows() == 0 || this->cols() == 0; }

    // - Access (mutable) -

    NodeId& prev_id(NodeId node_id) {
        assert(to_int(node_id) < this->cols());
        return this->prev_ids[to_int(node_id)];
    }

    NodeId& next_id(CallsiteId callsite_id, NodeId node_id) {
        assert(to_int(callsite_id) < this->rows());
        assert(to_int(node_id) < this->cols());
        return this->next_ids[to_int(callsite_id) + to_int(node_id) * this->rows_capacity];
    }

    duration& time(NodeId node_id) {
        assert(to_int(node_id) < this->cols());
        return this->times[to_int(node_id)];
    }

    CallsiteInfo& callsite(CallsiteId callsite_id) {
        assert(to_int(callsite_id) < this->rows());
        return this->callsites[to_int(callsite_id)];
    }

    // - Access (const) -

    const NodeId& prev_id(NodeId node_id) const {
        assert(to_int(node_id) < this->cols());
        return this->prev_ids[to_int(node_id)];
    }

    const NodeId& next_id(CallsiteId callsite_id, NodeId node_id) const {
        assert(to_int(callsite_id) < this->rows());
        assert(to_int(node_id) < this->cols());
        return this->next_ids[to_int(callsite_id) + to_int(node_id) * this->rows_capacity];
    }

    const duration& time(NodeId node_id) const {
        assert(to_int(node_id) < this->cols());
        return this->times[to_int(node_id)];
    }

    const CallsiteInfo& callsite(CallsiteId callsite_id) const {
        assert(to_int(callsite_id) < this->rows());
        return this->callsites[to_int(callsite_id)];
    }

    // - Resizing -

    void resize(std::size_t new_rows, std::size_t new_cols) {
        const bool new_rows_over_capacity = new_rows > this->rows_capacity;
        const bool new_cols_over_capacity = new_cols > this->cols_capacity;
        const bool requires_reallocation  = new_rows_over_capacity || new_cols_over_capacity;

        // No reallocation case
        if (!requires_reallocation) {
            this->rows_size = new_rows;
            this->cols_size = new_cols;
            return;
        }

        // Reallocate
        const std::size_t new_rows_capacity =
            new_rows_over_capacity ? new_rows + NodeMatrix::row_growth_add : this->rows_capacity;
        const std::size_t new_cols_capacity =
            new_cols_over_capacity ? new_cols * NodeMatrix::col_growth_mul : this->cols_capacity;

        array_type<NodeId>       new_prev_ids(new_cols_capacity, NodeId::empty);
        array_type<NodeId>       new_next_ids(new_rows_capacity * new_cols_capacity, NodeId::empty);
        array_type<duration>     new_times(new_cols_capacity, duration{});
        array_type<CallsiteInfo> new_callsites(new_rows_capacity, CallsiteInfo{});

        // Copy old data
        for (std::size_t j = 0; j < this->cols_size; ++j) new_prev_ids[j] = this->prev_ids[j];
        for (std::size_t j = 0; j < this->cols_size; ++j)
            for (std::size_t i = 0; i < this->rows_size; ++i)
                new_next_ids[i + j * new_rows_capacity] = this->next_ids[i + j * this->rows_capacity];
        for (std::size_t j = 0; j < this->cols_size; ++j) new_times[j] = this->times[j];
        for (std::size_t i = 0; i < this->rows_size; ++i) new_callsites[i] = this->callsites[i];

        // Assign new data
        this->prev_ids  = std::move(new_prev_ids);
        this->next_ids  = std::move(new_next_ids);
        this->times     = std::move(new_times);
        this->callsites = std::move(new_callsites);

        this->rows_size     = new_rows;
        this->cols_size     = new_cols;
        this->rows_capacity = new_rows_capacity;
        this->cols_capacity = new_cols_capacity;
    }

    void grow_callsites() { this->resize(this->rows_size + 1, this->cols_size); }

    void grow_nodes() { this->resize(this->rows_size, this->cols_size + 1); }

    template <class Func, std::enable_if_t<std::is_invocable_v<Func, CallsiteId, NodeId, std::size_t>, bool> = true>
    void node_apply_recursively(CallsiteId callsite_id, NodeId node_id, Func func, std::size_t depth) const {
        func(callsite_id, node_id, depth);

        for (std::size_t i = 0; i < this->rows(); ++i) {
            const CallsiteId next_callsite_id = CallsiteId(i);
            const NodeId     next_node_id     = this->next_id(next_callsite_id, node_id);
            if (next_node_id != NodeId::empty)
                this->node_apply_recursively(next_callsite_id, next_node_id, func, depth + 1);
        }
        // 'node_is' corresponds to a matrix column, to iterate over all
        // "next" nodes we iterate rows (callsites) in a column
    }

    template <class Func, std::enable_if_t<std::is_invocable_v<Func, CallsiteId, NodeId, std::size_t>, bool> = true>
    void root_apply_recursively(Func func) const {
        if (!this->rows_size || !this->cols_size) return; // possibly redundant

        func(CallsiteId::empty, NodeId::root, 0);

        for (std::size_t i = 0; i < this->rows(); ++i) {
            const CallsiteId next_callsite_id = CallsiteId(i);
            const NodeId     next_node_id     = this->next_id(next_callsite_id, NodeId::root);
            if (next_node_id != NodeId::empty) this->node_apply_recursively(next_callsite_id, next_node_id, func, 1);
        }
    }
};

// ================
// --- Profiler ---
// ================

struct ThreadLifetimeData {
    NodeMatrix mat;
    bool       joined = false;
};

struct ThreadIdData {
    std::vector<ThreadLifetimeData> lifetimes;
    std::size_t                     readable_id;
    // since we need a map from 'std::thread::id' to both lifetimes and human-readable id mappings,
    // and those maps would be accessed at the same time, it makes sense to instead avoid a second
    // map lookup and merge both values into a single struct
};

class Profiler {
    // header-inline, only one instance exists, this instance is effectively a persistent
    // "database" responsible for collecting & formatting results

    using call_graph_storage = std::unordered_map<std::thread::id, ThreadIdData>;
    // thread ID by itself is not enough to identify a distinct thread with a finite lifetime, OS only guarantees
    // unique thread ids for currently existing threads, new threads may reuse IDs of the old joined threads,
    // this is why for every thread ID we store a vector - this vector grows every time a new thread with a given
    // id is created

    friend struct ThreadCallGraph;

    call_graph_storage call_graph_info;
    std::mutex         call_graph_mutex;

    std::thread::id main_thread_id = std::this_thread::get_id();
    std::size_t     thread_counter = 0;

    bool       print_at_destruction = true;
    std::mutex setter_mutex;

    bool results_are_empty() {
        // A check used to deduce whether we have any meaningful results to automatically print after exit,
        // all threads joined, yet none of them contain any profiling records <=> no profiling was ever invoked
        for (const auto& [thread_id, thread_lifetimes] : this->call_graph_info)
            for (const auto& lifetime : thread_lifetimes.lifetimes)
                if (lifetime.joined == false || !lifetime.mat.empty()) return false;

        return true;
    }

    std::string format_available_results(const Style& style = Style{}) {
        const std::lock_guard lock(this->call_graph_mutex);

        std::vector<FormattedRow> rows;
        std::string               res;

        // Format header
        if (style.color) res += color::bold_cyan;
        append_fold(res, "\n-------------------- UTL PROFILING RESULTS ---------------------\n");
        if (style.color) res += color::reset;

        for (const auto& [thread_id, thread_lifetimes] : this->call_graph_info) {
            for (std::size_t reuse = 0; reuse < thread_lifetimes.lifetimes.size(); ++reuse) {
                const auto&       mat         = thread_lifetimes.lifetimes[reuse].mat;
                const bool        joined      = thread_lifetimes.lifetimes[reuse].joined;
                const std::size_t readable_id = thread_lifetimes.readable_id;

                rows.clear();
                rows.reserve(mat.cols());

                const std::string thread_str      = (readable_id == 0) ? "main" : std::to_string(readable_id);
                const bool        thread_uploaded = !mat.empty();

                // Format thread header
                if (style.color) res += color::bold_cyan;
                append_fold(res, "\n# Thread [", thread_str, "] (reuse ", std::to_string(reuse), ")");
                if (style.color) res += color::reset;

                // Format thread status
                if (style.color) res += joined ? color::bold_green : color::bold_magenta;
                append_fold(res, joined ? " (joined)" : " (running)");
                if (style.color) res += color::reset;

                // Early escape for lifetimes that haven't uploaded yet
                if (!thread_uploaded) {
                    append_fold(res, '\n');
                    continue;
                }

                // Format thread runtime
                const ms   runtime     = mat.time(NodeId::root);
                const auto runtime_str = format_number(runtime.count(), std::chars_format::fixed, 2);

                if (style.color) res += color::bold_blue;
                append_fold(res, " (runtime -> ", runtime_str, " ms)\n");
                if (style.color) res += color::reset;

                // Gather call graph data in a digestible format
                mat.root_apply_recursively([&](CallsiteId callsite_id, NodeId node_id, std::size_t depth) {
                    if (callsite_id == CallsiteId::empty) return;

                    const auto&  callsite   = mat.callsite(callsite_id);
                    const auto&  time       = mat.time(node_id);
                    const double percentage = time / runtime;

                    rows.push_back(FormattedRow{callsite, time, depth, percentage});
                });

                // Format call graph columns row by row
                std::vector<std::array<std::string, 4>> rows_str;
                rows_str.reserve(rows.size());

                for (const auto& row : rows) {
                    const auto percentage_num_str = format_number(row.percentage * 100, std::chars_format::fixed, 2);

                    auto percentage_str = std::string(style.indent * row.depth, ' ');
                    append_fold(percentage_str, " - ", percentage_num_str, "% ");

                    auto time_str     = format_number(ms(row.time).count(), std::chars_format::fixed, 2) + " ms";
                    auto label_str    = std::string(row.callsite.label);
                    auto callsite_str = format_call_site(row.callsite.file, row.callsite.line, row.callsite.func);

                    rows_str.push_back({std::move(percentage_str), std::move(time_str), std::move(label_str),
                                        std::move(callsite_str)});
                }

                // Gather column widths for alignment
                std::size_t width_percentage = 0, width_time = 0, width_label = 0, width_callsite = 0;
                for (const auto& row : rows_str) {
                    width_percentage = max(width_percentage, row[0].size());
                    width_time       = max(width_time, row[1].size());
                    width_label      = max(width_label, row[2].size());
                    width_callsite   = max(width_callsite, row[3].size());
                }

                assert(rows.size() == rows_str.size());

                // Format resulting string with colors & alignment
                for (std::size_t i = 0; i < rows.size(); ++i) {
                    const bool color_row_red     = style.color && rows[i].percentage > style.cutoff_red;
                    const bool color_row_yellow  = style.color && rows[i].percentage > style.cutoff_yellow;
                    const bool color_row_gray    = style.color && rows[i].percentage < style.cutoff_gray;
                    const bool color_was_applied = color_row_red || color_row_yellow || color_row_gray;

                    if (color_row_red) res += color::red;
                    else if (color_row_yellow) res += color::yellow;
                    else if (color_row_gray) res += color::gray;

                    append_aligned_left(res, rows_str[i][0], width_percentage, '-');
                    append_fold(res, " | ");
                    append_aligned_right(res, rows_str[i][1], width_time);
                    append_fold(res, " | ");
                    append_aligned_right(res, rows_str[i][2], width_label);
                    append_fold(res, " | ");
                    append_aligned_left(res, rows_str[i][3], width_callsite);
                    append_fold(res, " |");

                    if (color_was_applied) res += color::reset;

                    res += '\n';
                }
            }
        }

        return res;
    }

    void call_graph_add(std::thread::id thread_id) {
        const std::lock_guard lock(this->call_graph_mutex);

        const auto [it, emplaced] = this->call_graph_info.try_emplace(thread_id);

        // Emplacement took place =>
        // This is the first time inserting this thread id, add human-readable mapping that grows by 1 for each new
        // thread, additional checks are here to ensure that main thread is always '0' and other threads are always
        // '1+' even if main thread wasn't the first one to register a profiler. This is important because formatting
        // prints zero-thread as '[main]' and we don't want this title to go to some other thread
        if (emplaced) it->second.readable_id = (thread_id == this->main_thread_id) ? 0 : ++this->thread_counter;

        // Add a default-constructed call graph matrix to lifetimes,
        // - if this thread ID was emplaced      then this is a non-reused thread ID
        // - if this thread ID was already there then this is a     reused thread ID
        // regardless, our actions are the same
        it->second.lifetimes.emplace_back();
    }

    void call_graph_upload(std::thread::id thread_id, NodeMatrix&& info, bool joined) {
        const std::lock_guard lock(this->call_graph_mutex);

        auto& lifetime  = this->call_graph_info.at(thread_id).lifetimes.back();
        lifetime.mat    = std::move(info);
        lifetime.joined = joined;
    }

public:
    void upload_this_thread(); // depends on the 'ThreadCallGraph', defined later

    void print_at_exit(bool value) noexcept {
        const std::lock_guard lock(this->setter_mutex);
        // useless most of the time, but allows public API to be completely thread-safe

        this->print_at_destruction = value;
    }

    std::string format_results(const Style& style = Style{}) {
        this->upload_this_thread();
        // Call graph from current thread is not yet uploaded by its 'thread_local' destructor, we need to
        // explicitly pull it which we can easily do since current thread can't contest its own resources

        return this->format_available_results(style);
    }

    ~Profiler() {
        if (!this->print_at_destruction) return; // printing was manually disabled
        if (this->results_are_empty()) return;   // no profiling was ever invoked
        std::cout << format_available_results();
    }
};

inline Profiler profiler;

// =========================
// --- Thread Call Graph ---
// =========================

struct ThreadCallGraph {
    // header-inline-thread_local, gets created whenever we create a new thread anywhere,
    // this class is responsible for managing some thread-specific things on top of our
    // core graph traversal structure and provides an actual high-level API for graph traversal

    NodeMatrix      mat;
    NodeId          current_node_id  = NodeId::empty;
    time_point      entry_time_point = clock::now();
    std::thread::id thread_id        = std::this_thread::get_id();

    NodeId create_root_node() {
        const NodeId prev_node_id = this->current_node_id;
        this->current_node_id     = NodeId::root; // advance to a new node

        this->mat.grow_nodes();
        this->mat.prev_id(this->current_node_id) = prev_node_id;
        // link new node backwards, since this is a root the prev. one is empty and doesn't need to link forwards

        return this->current_node_id;
    }

    NodeId create_node(CallsiteId callsite_id) {
        const NodeId prev_node_id = this->current_node_id;
        this->current_node_id     = NodeId(this->mat.cols()); // advance to a new node

        this->mat.grow_nodes();
        this->mat.prev_id(this->current_node_id)     = prev_node_id;          // link new node backwards
        this->mat.next_id(callsite_id, prev_node_id) = this->current_node_id; // link prev. node forwards

        return this->current_node_id;
    }

    void upload_results(bool joined) {
        this->mat.time(NodeId::root) = clock::now() - this->entry_time_point;
        // root node doesn't get time updates from timers, we need to collect total runtime manually

        profiler.call_graph_upload(this->thread_id, NodeMatrix(this->mat), joined); // deep copy & mutex lock, slow
    }

public:
    ThreadCallGraph() {
        profiler.call_graph_add(this->thread_id);

        this->create_root_node();
    }

    ~ThreadCallGraph() { this->upload_results(true); }

    NodeId traverse_forward(CallsiteId callsite_id) {
        const NodeId next_node_id = this->mat.next_id(callsite_id, this->current_node_id);
        // 1 dense matrix lookup to advance the node forward, 1 branch to check its existence
        // 'callsite_id' is always valid due to callsite & timer initialization order

        // - node missing  => create new node and return its id
        if (next_node_id == NodeId::empty) return this->create_node(callsite_id);

        // - node exists   =>  return existing id
        return this->current_node_id = next_node_id;
    }

    void traverse_back() { this->current_node_id = this->mat.prev_id(this->current_node_id); }

    void record_time(duration time) { this->mat.time(this->current_node_id) += time; }

    CallsiteId callsite_add(const CallsiteInfo& info) { // adds new callsite & returns its id
        const CallsiteId new_callsite_id = CallsiteId(this->mat.rows());

        this->mat.grow_callsites();
        this->mat.callsite(new_callsite_id) = info;

        return new_callsite_id;
    }
};

inline thread_local ThreadCallGraph thread_call_graph;

inline void Profiler::upload_this_thread() { thread_call_graph.upload_results(false); }

// =======================
// --- Callsite Marker ---
// =======================

struct Callsite {
    // local-thread_local, small marker binding a numeric ID to a callsite

    CallsiteId callsite_id;

public:
    Callsite(const CallsiteInfo& info) { this->callsite_id = thread_call_graph.callsite_add(info); }

    CallsiteId get_id() const noexcept { return this->callsite_id; }
};

// =============
// --- Timer ---
// =============

class Timer {
    time_point entry = clock::now();

public:
    Timer(CallsiteId callsite_id) { thread_call_graph.traverse_forward(callsite_id); }

    void finish() const {
        thread_call_graph.record_time(clock::now() - this->entry);
        thread_call_graph.traverse_back();
    }
};

struct ScopeTimer : public Timer { // just like regular timer, but finishes at the end of the scope
    ScopeTimer(CallsiteId callsite_id) : Timer(callsite_id) {}

    constexpr operator bool() const noexcept { return true; }
    // allows us to use create scope timers inside 'if constexpr' & have applies-to-next-expression semantics for macro

    ~ScopeTimer() { this->finish(); }
};

} // namespace utl::profiler::impl

// =====================
// --- Helper macros ---
// =====================

#define utl_profiler_concat_tokens(a, b) a##b
#define utl_profiler_concat_tokens_wrapper(a, b) utl_profiler_concat_tokens(a, b)
#define utl_profiler_uuid(varname_) utl_profiler_concat_tokens_wrapper(varname_, __LINE__)
// creates token 'varname_##__LINE__' from 'varname_', necessary
// to work around some macro expansion order shenanigans

// ______________________ PUBLIC API ______________________

// ==========================================
// --- Definitions with profiling enabled ---
// ==========================================

namespace utl::profiler {

using impl::Profiler;
using impl::profiler;
using impl::Style;

} // namespace utl::profiler

#define UTL_PROFILER_SCOPE(label_)                                                                                     \
    constexpr bool utl_profiler_uuid(utl_profiler_macro_guard_) = true;                                                \
    static_assert(utl_profiler_uuid(utl_profiler_macro_guard_), "UTL_PROFILER is a multi-line macro.");                \
                                                                                                                       \
    const thread_local utl::profiler::impl::Callsite utl_profiler_uuid(utl_profiler_callsite_)(                        \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    const utl::profiler::impl::ScopeTimer utl_profiler_uuid(utl_profiler_scope_timer_) {                               \
        utl_profiler_uuid(utl_profiler_callsite_).get_id()                                                             \
    }

// all variable names are concatenated with a line number to prevent shadowing when then there are multiple nested
// profilers, this isn't a 100% foolproof solution, but it works reasonably well. Shadowed variables don't have any
// effect on functionality, but might cause warnings from some static analysis tools
//
// 'constexpr bool' and 'static_assert()' are here to improve error messages when this macro is misused as an
// expression, when someone writes 'if (...) UTL_PROFILER_SCOPE(...) func()' instead of many ugly errors they
// will see a macro expansion that contains a 'static_assert()' with a proper message

#define UTL_PROFILER(label_)                                                                                           \
    constexpr bool utl_profiler_uuid(utl_profiler_macro_guard_) = true;                                                \
    static_assert(utl_profiler_uuid(utl_profiler_macro_guard_), "UTL_PROFILER is a multi-line macro.");                \
                                                                                                                       \
    const thread_local utl::profiler::impl::Callsite utl_profiler_uuid(utl_profiler_callsite_)(                        \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    if constexpr (const utl::profiler::impl::ScopeTimer utl_profiler_uuid(utl_profiler_scope_timer_){                  \
                      utl_profiler_uuid(utl_profiler_callsite_).get_id()})

// 'if constexpr (timer)' allows this macro to "capture" the scope of the following expression

#define UTL_PROFILER_BEGIN(segment_, label_)                                                                           \
    const thread_local utl::profiler::impl::Callsite utl_profiler_callsite_##segment_(                                 \
        utl::profiler::impl::CallsiteInfo{__FILE__, __func__, label_, __LINE__});                                      \
                                                                                                                       \
    const utl::profiler::impl::Timer utl_profiler_timer_##segment_ { utl_profiler_callsite_##segment_.get_id() }

#define UTL_PROFILER_END(segment_) utl_profiler_timer_##segment_.finish()

// ===========================================
// --- Definitions with profiling disabled ---
// ===========================================

// No-op mocks of the public API and minimal necessary includes

#else

#include <cstddef> // size_t
#include <string>  // string

namespace utl::profiler {
struct Style {
    std::size_t indent = 2;
    bool        color  = true;

    double cutoff_red    = 0.40;
    double cutoff_yellow = 0.20;
    double cutoff_gray   = 0.01;
};

struct Profiler {
    void print_at_exit(bool) noexcept {}

    void upload_this_thread() {}

    std::string format_results(const Style = Style{}) { return "<profiling is disabled>"; }
};
} // namespace utl::profiler

#define UTL_PROFILER_SCOPE(label_) static_assert(true)
#define UTL_PROFILER(label_)
#define UTL_PROFILER_BEGIN(segment_, label_) static_assert(true)
#define UTL_PROFILER_END(segment_) static_assert(true)

// 'static_assert(true)' emulates the "semicolon after the macro" requirement

#endif

#endif
#endif // module utl::profiler






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::progressbar
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_progressbar.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PROGRESSBAR)
#ifndef UTLHEADERGUARD_PROGRESSBAR
#define UTLHEADERGUARD_PROGRESSBAR

#define UTL_PROGRESSBAR_VERSION_MAJOR 1
#define UTL_PROGRESSBAR_VERSION_MINOR 0
#define UTL_PROGRESSBAR_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <algorithm>   // max(), clamp()
#include <array>       // array
#include <charconv>    // to_chars
#include <chrono>      // chrono::steady_clock, chrono::time_point<>, chrono::duration_cast<>
#include <cstddef>     // size_t
#include <iostream>    // cout
#include <iterator>    // ostream_iterator<>
#include <string>      // string
#include <string_view> // string_view

// ____________________ DEVELOPER DOCS ____________________

// Simple progress bars for terminal applications. Rendered in ASCII on the main thread with manual updates
// for maximal compatibility. Perhaps can be extended with some fancier async options that display animations.
//
// Used to be implemented in terms of 'std::stringstream' but later got rewritten to improve performance,
// reduce includes, allow more style configuration and make API more robust against misuse.

// ____________________ IMPLEMENTATION ____________________

namespace utl::progressbar::impl {

// Proper progress bar, uses '\r' to render new state in the same spot.
// Allocates when formatting things for the first time, after that storage gets reused.
class Percentage {
public:
    // - Public parameters -
    struct Style {
        char        fill            = '#';
        char        empty           = '.';
        char        left            = '[';
        char        right           = ']';
        std::string estimate_prefix = "(remaining: ";
        std::string estimate_suffix = ")";
    } style;

    bool show_bar        = true;
    bool show_percentage = true;
    bool show_estimate   = true;

    std::size_t bar_length  = 30;
    double      update_rate = 2.5e-3; // every quarter of a % feels like a good default

    // - Public API -
    Percentage() : start_time_point(clock::now()) {
        std::cout << '\n';
        this->draw();
        std::cout.flush();
    }

    void set_progress(double value) {
        value = std::clamp(value, 0., 1.);

        if (value - this->progress < this->update_rate) return; // prevents progress decrement

        this->progress = value;

        this->draw();
        std::cout.flush();
    }

    void finish() {
        if (this->finished) return; // prevents weird formatting from multiple 'finish()' calls

        this->progress = 1.;
        this->finished = true;

        this->draw();
        std::cout << '\n';
        std::cout.flush();
    }

    void update_style() {
        this->draw();
        std::cout.flush();
    }

private:
    // - Internal state -
    using clock = std::chrono::steady_clock;

    clock::time_point start_time_point = clock::now();
    std::size_t       max_drawn_length = 0;
    double            progress         = 0;
    bool              finished         = false;

    std::string buffer; // keep the buffer so we don't have to reallocate each time

    void format_bar() {
        if (!this->show_bar) return;

        const std::size_t fill_length  = static_cast<std::size_t>(this->progress * this->bar_length);
        const std::size_t empty_length = this->bar_length - fill_length;

        this->buffer += this->style.left;
        this->buffer.append(fill_length, this->style.fill);
        this->buffer.append(empty_length, this->style.empty);
        this->buffer += this->style.right;
        this->buffer += ' ';
    }

    void format_percentage() {
        if (!this->show_percentage) return;

        constexpr auto        format    = std::chars_format::fixed;
        constexpr std::size_t precision = 2;
        constexpr std::size_t max_chars = 6; // enough for for '0.xx' to '100.xx',

        std::array<char, max_chars> chars;
        const double                percentage = this->progress * 100; // 'set_progress()' enforces 0 <= progress <= 1

        const auto end_ptr = std::to_chars(chars.data(), chars.data() + max_chars, percentage, format, precision).ptr;
        // can't error, buffer size is guaranteed to be enough

        this->buffer.append(chars.data(), end_ptr - chars.data());
        this->buffer += '%';
        this->buffer += ' ';
    }

    void format_estimate() {
        if (!this->show_estimate) return;
        if (!this->progress) return;

        const auto elapsed  = clock::now() - this->start_time_point;
        const auto estimate = elapsed * (1. - this->progress) / this->progress;

        const auto hours   = std::chrono::duration_cast<std::chrono::hours>(estimate);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(estimate - hours);
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(estimate - hours - minutes);

        this->buffer += this->style.estimate_prefix;

        if (hours.count()) {
            this->buffer += std::to_string(hours.count());
            this->buffer += " hours ";
            this->buffer += std::to_string(minutes.count());
            this->buffer += " min ";
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        } else if (minutes.count()) {
            this->buffer += std::to_string(minutes.count());
            this->buffer += " min ";
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        } else {
            this->buffer += std::to_string(seconds.count());
            this->buffer += " sec";
        }

        this->buffer += this->style.estimate_suffix;
    }

    void draw() {
        this->buffer.clear();
        this->buffer += '\r';

        // Draw progressbar
        this->format_bar();
        this->format_percentage();
        this->format_estimate();

        // Draw spaces over potential remains of the previous bar (which could be longer due to time estimate)
        this->max_drawn_length = std::max(this->max_drawn_length, this->buffer.size());
        this->buffer.append(this->max_drawn_length - this->buffer.size(), ' ');

        std::cout << this->buffer;
    }
};

// Minimalistic progress bar, used when terminal doesn't support '\r' (they exist).
// Does not allocate.
class Ruler {
    constexpr static std::string_view ticks      = "0    10   20   30   40   50   60   70   80   90   100%";
    constexpr static std::string_view ruler      = "|----|----|----|----|----|----|----|----|----|----|";
    constexpr static std::size_t      bar_length = ruler.size();

public:
    // - Public parameters -
    struct Style {
        char fill            = '#';
        char ruler_line      = '-';
        char ruler_delimiter = '|';
    } style;

    bool show_ticks = true;
    bool show_ruler = true;
    bool show_bar   = true; // useless, but might as well have it for uniformity

    // - Public API -
    Ruler() {
        std::cout << '\n';
        this->draw_ticks();
        std::cout << '\n';
        this->draw_ruler();
        std::cout << '\n';
        std::cout.flush();
    }

    void set_progress(double value) {
        value = std::clamp(value, 0., 1.);

        this->progress_in_chars = static_cast<std::size_t>(this->bar_length * value);

        this->draw_bar();
        std::cout.flush();
    }

    void finish() {
        if (this->finished) return; // prevents weird formatting from multiple 'finish()' calls

        this->progress_in_chars = this->bar_length;
        this->finished          = true;

        this->draw_bar();
        std::cout << '\n';
        std::cout.flush();
    }

private:
    // - Internal state -
    std::size_t progress_in_chars = 0;
    std::size_t chars_drawn       = 0;
    bool        finished          = false;

    void draw_ticks() {
        if (!this->show_ticks) return;
        std::cout << this->ticks;
    }

    void draw_ruler() {
        if (!this->show_ruler) return;

        std::array<char, ruler.size()> buffer;
        for (std::size_t i = 0; i < ruler.size(); ++i)
            buffer[i] = (this->ruler[i] == '|') ? this->style.ruler_delimiter : this->style.ruler_line;
        // formats ruler without allocating

        std::cout.write(buffer.data(), buffer.size());
    }

    void draw_bar() {
        if (!this->show_bar) return;

        if (this->progress_in_chars > this->chars_drawn)
            std::fill_n(std::ostream_iterator<char>(std::cout), this->progress_in_chars - this->chars_drawn,
                        this->style.fill);

        this->chars_drawn = this->progress_in_chars;
    }
};

} // namespace utl::progressbar::impl

// ______________________ PUBLIC API ______________________

namespace utl::progressbar {

using impl::Percentage;
using impl::Ruler;

} // namespace utl::progressbar

#endif
#endif // module utl::progressbar






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::random
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_random.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_RANDOM)
#ifndef UTLHEADERGUARD_RANDOM
#define UTLHEADERGUARD_RANDOM

#define UTL_RANDOM_VERSION_MAJOR 2
#define UTL_RANDOM_VERSION_MINOR 1
#define UTL_RANDOM_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <array>            // array<>
#include <cassert>          // assert()
#include <chrono>           // high_resolution_clock
#include <cstdint>          // uint8_t, uint16_t, uint32_t, uint64_t
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <mutex>            // mutex, lock_guard<>
#include <random>           // random_device, uniform_..._distribution<>, generate_canonical<>, seed_seq<>
#include <thread>           // thread::id, this_thread::get_id()
#include <type_traits>      // enable_if_t<>, is_integral<>, is_unsigned<>, is_floating_point<>
#include <utility>          // declval<>()
#include <vector>           // vector<>, hash<>

// ____________________ DEVELOPER DOCS ____________________

// Several <random> compatible PRNGs, slightly improved re-implementations of uniform distributions,
// "better" entropy sources and several convenience wrappers for rng.
//
// Everything implemented here should be portable assuming reasonable assumptions (like existence of
// uint32_t, uint64_t, 8-bit bytes, 32-bit floats, 64-bit doubles and etc.) which hold for most platforms.
//
// Optional macros:
// - #define UTL_RANDOM_USE_INTRINSICS // use rdtsc timestamps for entropy

// ____________________ IMPLEMENTATION ____________________

// ==================================
// --- Optional __rdtsc() support ---
// ==================================

#ifdef UTL_RANDOM_USE_INTRINSICS

// x86 RDTSC timestamps make for a good source of entropy
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define utl_random_cpu_counter __rdtsc()

#else

// Fallback onto a constant that changes with each compilation, not good, but better than nothing
#include <string> // string
#define utl_random_cpu_counter std::hash<std::string>{}(std::string(__TIME__))

#endif

namespace utl::random::impl {

// ============================
// --- SFINAE & type traits ---
// ============================

#define utl_random_define_trait(trait_name_, ...)                                                                      \
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
    using require_##trait_name_ = std::enable_if_t<trait_name_<T>::value, bool>


utl_random_define_trait(is_seed_seq,
                        std::declval<T>().generate(std::declval<std::uint32_t*>(), std::declval<std::uint32_t*>()));
// this type trait is necessary to restrict template constructors & seed functions that take 'SeedSeq&& seq',
// otherwise they will get picked instead of regular seeding methods even for integer arguments.
// This is how standard library seems to do it (based on GCC implementation) so we follow their API.

#undef utl_random_define_trait

template <class>
constexpr bool always_false_v = false;

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_bool = require<std::is_same_v<T, bool>>;

template <class T>
using require_not_bool = require<!std::is_same_v<T, bool>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

// ====================
// --- 128-bit uint ---
// ====================

// We need 128-bit uint for Lemire's uniform integer distribution algorithm,
// some PRNGs can also make use of wide integers

// GCC & clang provide 128-bit integers as compiler extension
#if defined(__SIZEOF_INT128__) && !defined(__wasm__)
using Uint128 = __uint128_t;

// Otherwise fallback onto either MSVC intrinsics or manual emulation
#else

// Emulation of 128-bit unsigned integer tailored specifically for usage in 64-bit Lemire's algorithm,
// this allows us to skip a lot of generic logic since we really only need 3 things:
//
//    1) 'uint128(x) * uint128(y)'       that performs 64x64 -> 128 bit multiplication
//    2) 'static_cast<std::uint64_t>(x)' that returns lower 64 bits
//    3) 'x >> 64'                       that returns upper 64 bits
//
struct Uint128 {
    std::uint64_t low{}, high{};

    constexpr Uint128(std::uint64_t low) noexcept : low(low) {}
    constexpr explicit Uint128(std::uint64_t low, std::uint64_t high) noexcept : low(low), high(high) {}

    [[nodiscard]] constexpr operator std::uint64_t() const noexcept { return this->low; }

    [[nodiscard]] constexpr Uint128 operator*(Uint128 other) const noexcept {
#if defined(UTL_RANDOM_USE_INTRINSICS) && defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        // Unlike GCC, MSVC also requires 'UTL_RANDOM_USE_INTRINSICS' flag since it also needs '#include <intrin.h>'
        // for 128-bit multiplication, which could be considered a somewhat intrusive thing to include

        std::uint64_t upper = 0;
        std::uint64_t lower = _umul128(this->low, other.low, &upper);

        return Uint128{lower, upper};

#else

        // Compute all of the cross products
        const std::uint64_t lo_lo = (this->low & 0xFFFFFFFF) * (other.low & 0xFFFFFFFF);
        const std::uint64_t hi_lo = (this->low >> 32) * (other.low & 0xFFFFFFFF);
        const std::uint64_t lo_hi = (this->low & 0xFFFFFFFF) * (other.low >> 32);
        const std::uint64_t hi_hi = (this->low >> 32) * (other.low >> 32);

        // Add products together, this will never overflow
        const std::uint64_t cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
        const std::uint64_t upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
        const std::uint64_t lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

        return Uint128{lower, upper};

#endif
    }

    [[nodiscard]] constexpr Uint128 operator>>(int) const noexcept { return this->high; }
};

#endif

// clang-format off
template<class T> struct wider { static_assert(always_false_v<T>, "Missing specialization."); };

template<> struct wider<std::uint8_t > { using type = std::uint16_t; };
template<> struct wider<std::uint16_t> { using type = std::uint32_t; };
template<> struct wider<std::uint32_t> { using type = std::uint64_t; };
template<> struct wider<std::uint64_t> { using type = Uint128;       };

template<class T> using wider_t = typename wider<T>::type;
// clang-format on

// ===========================
// --- Bit-twiddling utils ---
// ===========================

template <class T, require_uint<T> = true>
[[nodiscard]] constexpr T uint_minus(T value) noexcept {
    return ~value + T(1);
    // MSVC with '/W2' warning level gives a warning when using unary minus with an unsigned value, this warning
    // gets elevated to a compilation error by '/sdl' flag, see
    // https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4146
    //
    // This is a case of MSVC not being standard-compliant, as unsigned '-x' is a perfectly defined operation which
    // evaluates to the same thing as '~x + 1u'. To work around such warning we define this function
}

// Merging integers into the bits of a larger one
[[nodiscard]] constexpr std::uint64_t merge_uint32_into_uint64(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::uint64_t>(a) | (static_cast<std::uint64_t>(b) << 32);
}

// Helper method to crush large uints to uint32_t,
// inspired by Melissa E. O'Neil's randutils https://gist.github.com/imneme/540829265469e673d045
template <class T, require_integral<T> = true, require<sizeof(T) <= 8> = true>
[[nodiscard]] constexpr std::uint32_t crush_to_uint32(T value) noexcept {
    if constexpr (sizeof(value) <= 4) {
        return std::uint32_t(value);
    } else {
        const std::uint64_t res = static_cast<std::uint64_t>(value) * 0xbc2ad017d719504d;
        return static_cast<std::uint32_t>(res ^ (res >> 32));
    }
}

// Seed sequence helpers
template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
std::uint32_t seed_seq_to_uint32(SeedSeq&& seq) {
    std::array<std::uint32_t, 1> temp;
    seq.generate(temp.begin(), temp.end());
    return temp[0];
}

template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
std::uint64_t seed_seq_to_uint64(SeedSeq&& seq) {
    std::array<std::uint32_t, 2> temp;
    seq.generate(temp.begin(), temp.end());
    return merge_uint32_into_uint64(temp[0], temp[1]);
}

// 'std::rotl()' from C++20, used by many PRNGs,
// have to use long name because platform-specific includes declare '_rotl' as a macro
template <class T, require_uint<T> = true>
[[nodiscard]] constexpr T rotl(T x, int k) noexcept {
    return (x << k) | (x >> (std::numeric_limits<T>::digits - k));
}

// Some generators shouldn't be zero initialized, in a perfect world the user would never
// do that, but in case they happened to do so regardless we can remap 0 to some "weird"
// value that isn't like to intersect with any other seeds generated by the user. Rejecting
// zero seeds completely wouldn't be appropriate for compatibility reasons.
template <class T, std::size_t N, require_uint<T> = true>
[[nodiscard]] constexpr bool is_zero_state(const std::array<T, N>& state) {
    for (const auto& e : state)
        if (e != T(0)) return false;
    return true;
}

template <class ResultType>
[[nodiscard]] constexpr ResultType mix_seed(ResultType seed) {
    std::uint64_t state = (static_cast<std::uint64_t>(seed) + 0x9E3779B97f4A7C15);
    state               = (state ^ (state >> 30)) * 0xBF58476D1CE4E5B9;
    state               = (state ^ (state >> 27)) * 0x94D049BB133111EB;
    return static_cast<ResultType>(state ^ (state >> 31));
    // some of the 16/32-bit PRNGs have bad correlation on the successive seeds, this can  usually be alleviated
    // by adding a single iteration of a "good" PRNG to pre-mix the seed, here we have an iteration of SplitMix64
}

template <class T, require_uint<T> = true>
constexpr T default_seed = std::numeric_limits<T>::max() / 2 + 1;
// an "overall decent" default seed - doesn't have too many zeroes,
// unlikely to accidentally match with a user-defined seed


// =========================
// --- Random Generators ---
// =========================

// Implementation of several "good" PRNGS.
//
// All generators meets uniform random number generator requirements
// (C++17 and below, see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
// (C++20 and above, see https://en.cppreference.com/w/cpp/numeric/random/uniform_random_bit_generator)

// Note:
// Here PRNGs take 'SeedSeq' as a forwarding reference 'SeedSeq&&', while standard PRNGS take 'SeedSeq&',
// this is how it should've been done in the standard too, but for some reason they only standardized
// l-value references, perfect forwarding probably just wasn't in use at the time.

namespace generators {

// --- 16-bit PRNGs ---
// --------------------

// Implementation of 16-bit Romu Mono engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       4 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "RomuMono16" is the fastest 16-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 16-bit PRNG, so if speed is needed at all costs, this one provides it.
//
class RomuMono16 {
public:
    using result_type = std::uint16_t;

private:
    std::uint32_t s{}; // notice 32-bit value as a state rather than two 16-bit ints

public:
    constexpr explicit RomuMono16(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuMono16(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s = (seed & 0x1fffffffu) + 1156979152u; // accepts 29 seed-bits

        for (std::size_t i = 0; i < 10; ++i) this->operator()();
        // naively seeded RomuMono produces correlating patterns on the first iterations
        // for successive seeds, we can do a few iterations to escape that
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint32(seq);

        if (this->s == 0) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = this->s >> 16;
        this->s *= 3611795771u;
        this->s = rotl(this->s, 12);
        return result;
    }
};

// --- 32-bit PRNGs ---
// --------------------

// Implementation of 32-bit splitmix adopted from MurmurHash3, based on paper by Guy L. Steele,
// Doug Lea, and Christine H. Flood. 2014. "Fast splittable pseudorandom number generators"
// see http://marc-b-reynolds.github.io/shf/2017/09/27/LPRNS.html
//     https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf
//     https://github.com/umireon/my-random-stuff/blob/e7b17f992955f4dbb02d4016682113b48b2f6ec1/xorshift/splitmix32.c
//
// Performance: Excellent
// Quality:     3/5
// State:       4 bytes
//
// One of the fastest 32-bit generators that requires only a single 'std::uint32_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
// 32-bit version is somewhat lacking in terms of quality estimate data (relative to the widely used
// 64-bit version), however it still seems to be quite decent.
//
class SplitMix32 {
public:
    using result_type = std::uint32_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix32(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit SplitMix32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s = mix_seed(seed);
        // naively seeded SplitMix32 has a horrible correlation between successive seeds, we can mostly alleviate
        // the issue by pre-mixing the seed with a single iteration of a "better" 64-bit algorithm
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint32(seq);
    }

    constexpr result_type operator()() noexcept {
        result_type result = (this->s += 0x9e3779b9);
        result             = (result ^ (result >> 16)) * 0x21f0aaad;
        result             = (result ^ (result >> 15)) * 0x735a2d97;
        return result ^ (result >> 15);
    }
};

// Implementation of Xoshiro128++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       16 bytes
//
// Excellent choice as a general purpose 32-bit PRNG.
// Battle-tested and provides a good statistical quality at an excellent speed.
//
class Xoshiro128PP {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro128PP(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit Xoshiro128PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = rotl(this->s[0] + this->s[3], 7) + this->s[0];
        const result_type t      = s[1] << 9;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = rotl(this->s[3], 11);
        return result;
    }
};

// Implementation of 32-bit Romu Trio engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       12 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "RomuTrio" is the fastest 32-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 32-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuTrio32 {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 3> s{};

public:
    constexpr explicit RomuTrio32(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuTrio32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix32 to initialize its state
        this->s[2] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type xp = this->s[0], yp = this->s[1], zp = this->s[2];
        this->s[0] = 3323815723u * zp;
        this->s[1] = yp - xp;
        this->s[1] = rotl(this->s[1], 6);
        this->s[2] = zp - yp;
        this->s[2] = rotl(this->s[2], 22);
        return xp;
    }
};

// --- 64-bit PRNGs ---
// --------------------

// Implementation of fixed-increment version of Java 8's SplittableRandom generator SplitMix64,
// see http://dx.doi.org/10.1145/2714064.2660195
//     http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
//     https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
//
// Performance: Excellent
// Quality:     4/5
// State:       8 bytes
//
// One of the fastest generators passing BigCrush that requires only a single 'std::uint64_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
//
class SplitMix64 {
public:
    using result_type = std::uint64_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix64(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit SplitMix64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept { this->s = seed; }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint64(seq);
    }

    constexpr result_type operator()() noexcept {
        std::uint64_t result = (this->s += 0x9E3779B97f4A7C15);
        result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        return result ^ (result >> 31);
    }
};

// Implementation of Xoshiro256++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       32 bytes
//
// Excellent choice as a general purpose PRNG.
// Used by several modern languages as their default.
//
class Xoshiro256PP {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro256PP(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit Xoshiro256PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = seed_seq_to_uint64(seq); // since seed_seq produces 32-bit ints,
        this->s[1] = seed_seq_to_uint64(seq); // we have to generate multiple and then
        this->s[2] = seed_seq_to_uint64(seq); // join them into std::uint64_t's to
        this->s[3] = seed_seq_to_uint64(seq); // properly initialize the entire state

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = rotl(this->s[0] + this->s[3], 23) + this->s[0];
        const result_type t      = this->s[1] << 17;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = rotl(this->s[3], 45);
        return result;
    }
};

// Implementation of Romu DuoJr engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       16 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "DuoJr" is the fastest 64-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 64-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuDuoJr64 {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 2> s{};

public:
    constexpr explicit RomuDuoJr64(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuDuoJr64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix64 to initialize its state
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = seed_seq_to_uint64(seq); // seed_seq returns 32-bit ints, we have to generate
        this->s[1] = seed_seq_to_uint64(seq); // multiple to initialize full state of 64-bit values

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type res = this->s[0];
        this->s[0]            = 15241094284759029579u * this->s[1];
        this->s[1]            = this->s[1] - res;
        this->s[1]            = rotl(this->s[1], 27);
        return res;
    }
};

// --- CSPRNGs ---
// ---------------

// Implementation of ChaCha20 CSPRNG conforming to RFC 7539 standard
// see https://datatracker.ietf.org/doc/html/rfc7539
//     https://www.rfc-editor.org/rfc/rfc7539#section-2.4
//     https://en.wikipedia.org/wiki/Salsa20

// Quarter-round operation for ChaCha20 stream cipher
constexpr void quarter_round(std::uint32_t& a, std::uint32_t& b, std::uint32_t& c, std::uint32_t& d) {
    a += b, d ^= a, d = rotl(d, 16);
    c += d, b ^= c, b = rotl(b, 12);
    a += b, d ^= a, d = rotl(d, 8);
    c += d, b ^= c, b = rotl(b, 7);
}

template <std::size_t rounds>
[[nodiscard]] constexpr std::array<std::uint32_t, 16> chacha_rounds(const std::array<std::uint32_t, 16>& input) {
    auto state = input;

    static_assert(rounds % 2 == 0, "ChaCha rounds happen in pairs, total number should be divisible by 2.");

    constexpr std::size_t alternating_round_pairs = rounds / 2;
    // standard number of ChaCha rounds as per RFC 7539 is 20 (ChaCha20 variation),
    // however there is a strong case for ChaCha12 being a more sensible default,
    // at the moment the upper bound of what seems "crackable" is somewhere around 7 rounds,
    // which is why ChaCha8 is also widely used whenever speed is necessary.

    for (std::size_t i = 0; i < alternating_round_pairs; ++i) {
        // Column rounds
        quarter_round(state[0], state[4], state[8], state[12]);
        quarter_round(state[1], state[5], state[9], state[13]);
        quarter_round(state[2], state[6], state[10], state[14]);
        quarter_round(state[3], state[7], state[11], state[15]);

        // Diagonal rounds
        quarter_round(state[0], state[5], state[10], state[15]);
        quarter_round(state[1], state[6], state[11], state[12]);
        quarter_round(state[2], state[7], state[8], state[13]);
        quarter_round(state[3], state[4], state[9], state[14]);
    }

    for (std::size_t i = 0; i < state.size(); ++i) state[i] += input[i];
    return state;
}

template <std::size_t rounds>
class ChaCha {
public:
    using result_type = std::uint32_t;

private:
    // Initial state components
    std::array<result_type, 8> key{};     // 256-bit key
    std::array<result_type, 3> nonce{};   // 96-bit nonce
    std::uint32_t              counter{}; // 32-bit counter

    // Block
    std::array<result_type, 16> block{};    // holds next 16 random numbers
    std::size_t                 position{}; // current position in the block

    constexpr static std::array<result_type, 4> constant = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    // "Magic constants" for ChaCha20 are defined through bit representations of the following char arrays:
    // { "expa", "nd 3", "2-by", "te k" },
    // what we have here is exactly that except written as 'std::uint32_t'

    constexpr void generate_new_block() {
        // Set ChaCha20 initial state as per RFC 7539
        //
        //          [ const   const const const ]
        // State    [ key     key   key   key   ]
        // matrix = [ key     key   key   key   ]
        //          [ counter nonce nonce nonce ]
        //
        const std::array<std::uint32_t, 16> input = {
            this->constant[0], this->constant[1], this->constant[2], this->constant[3], //
            this->key[0],      this->key[1],      this->key[2],      this->key[3],      //
            this->key[4],      this->key[5],      this->key[6],      this->key[7],      //
            this->counter,     this->nonce[0],    this->nonce[1],    this->nonce[2]     //
        };

        // Fill new block
        this->block = chacha_rounds<rounds>(input);
        ++this->counter;
    }

public:
    constexpr explicit ChaCha(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit ChaCha(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) {
        // Use some other PRNG to setup initial state
        SplitMix32 splitmix{seed};
        for (auto& e : this->key) e = splitmix();
        for (auto& e : this->nonce) e = splitmix();
        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        // Seed sequence allows user to introduce more entropy into the state

        seq.generate(this->key.begin(), this->key.end());
        seq.generate(this->nonce.begin(), this->nonce.end());

        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    constexpr result_type operator()() noexcept {
        // Generate new block if necessary
        if (this->position >= 16) {
            this->generate_new_block();
            this->position = 0;
        }

        // Get random value from the block and advance position cursor
        return this->block[this->position++];
    }
};

using ChaCha8  = ChaCha<8>;
using ChaCha12 = ChaCha<12>;
using ChaCha20 = ChaCha<20>;


} // namespace generators

// ===============
// --- Entropy ---
// ===============

inline std::seed_seq entropy_seq() {
    // Ensure thread safety of our entropy source, it should generally work fine even without
    // it, but with this we can be sure things never race
    static std::mutex     entropy_mutex;
    const std::lock_guard entropy_guard(entropy_mutex);

    // Hardware entropy (if implemented),
    // some platforms (mainly MinGW) implements random device as a regular PRNG that
    // doesn't change from run to run, this is horrible, but we can somewhat improve
    // things by mixing other sources of entropy. Since hardware entropy is a rather
    // limited resource we only call it once.
    static std::uint32_t          seed_rd = std::random_device{}();
    // after that we just scramble it with a regular PRNG
    static generators::SplitMix32 splitmix{seed_rd};
    seed_rd = splitmix();

    // Time in nanoseconds (on some platforms microseconds)
    const auto seed_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Heap address (tends to be random each run on most platforms)
    std::vector<std::uint32_t> dummy_vec(1, seed_rd);
    const std::size_t          heap_address_hash = std::hash<std::uint32_t*>{}(dummy_vec.data());

    // Stack address (also tends to be random)
    const std::size_t stack_address_hash = std::hash<decltype(heap_address_hash)*>{}(&heap_address_hash);

    // CPU counter (if available, hashed compilation time otherwise)
    const auto cpu_counter = static_cast<std::uint64_t>(utl_random_cpu_counter);

    // Thread ID (guarantees different seeding for thread-local PRNGs)
    const std::size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());

    // Note:
    // There are other sources of entropy, such as function addresses,
    // but those can be rather "constant" on some platforms

    return {
        seed_rd,                             //
        crush_to_uint32(seed_time),          //
        crush_to_uint32(heap_address_hash),  //
        crush_to_uint32(stack_address_hash), //
        crush_to_uint32(cpu_counter),        //
        crush_to_uint32(thread_id)           //
    };
}

inline std::uint32_t entropy() {
    auto seq = entropy_seq();
    return seed_seq_to_uint32(seq);
    // returns 'std::uint32_t' to mimic the return type of 'std::random_device', if we return uint64_t
    // brace-initializers will complain about narrowing conversion on some generators. If someone want
    // more entropy than that they can always use the whole sequence as a generic solution.
    // Also having one 'random::entropy()' is much nicer than 'random::entropy_32()' & 'random::entropy_64()'.
}

// =====================
// --- Distributions ---
// =====================

// --- Uniform int distribution ---
// --------------------------------

template <class T, class Gen, require_uint<T> = true>
constexpr T uniform_uint_lemire(Gen& gen, T range) noexcept(noexcept(gen())) {
    using W = wider_t<T>;

    W product = W(gen()) * W(range);
    T low     = T(product);
    if (low < range) {
        while (low < uint_minus(range) % range) {
            product = W(gen()) * W(range);
            low     = T(product);
        }
    }
    return product >> std::numeric_limits<T>::digits;
}

// Reimplementation of libc++ 'std::uniform_int_distribution<>' except
// - constexpr
// - const-qualified (relative to distribution parameters)
// - noexcept as long as 'Gen::operator()' is noexcept, which is true for all generators in this module
// - supports 'std::uint8_t', 'std::int8_t', 'char'
// - produces the same sequence on each platform
// Performance is exactly the same a libc++ version of 'std::uniform_int_distribution<>',
// in fact, it is likely to return the exact same sequence for most types
template <class T, class Gen, require_integral<T> = true>
constexpr T generate_uniform_int(Gen& gen, T min, T max) noexcept {
    using result_type    = T;
    using unsigned_type  = std::make_unsigned_t<result_type>;
    using generated_type = typename Gen::result_type;
    using common_type    = std::common_type_t<unsigned_type, generated_type>;

    constexpr common_type prng_min = Gen::min();
    constexpr common_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr common_type prng_range = prng_max - prng_min;
    constexpr common_type type_range = std::numeric_limits<common_type>::max();
    const common_type     range      = common_type(max) - common_type(min);

    common_type res{};

    // PRNG has enough state for the range
    if (prng_range > range) {
        const common_type ext_range = range + 1; // never overflows due to branch condition

        // PRNG is bit-uniform
        // => use Lemire's algorithm, GCC/clang provide 128-bit arithmetics natively, other
        //    compilers use emulation, Lemire with emulated 128-bit ints performs about the
        //    same as Java's "modx1", which is the best algorithm without wide arithmetics
        if constexpr (prng_range == type_range) {
            res = uniform_uint_lemire<common_type>(gen, ext_range);
        }
        // PRNG is non-uniform (usually because 'prng_min' is '1')
        // => fallback onto a 2-division algorithm
        else {
            const common_type scaling = prng_range / ext_range;
            const common_type past    = ext_range * scaling;

            do { res = common_type(gen()) - prng_min; } while (res >= past);
            res /= scaling;
        }
    }
    // PRNG needs several invocations to acquire enough state for the range
    else if (prng_range < range) {
        common_type temp{};
        do {
            constexpr common_type ext_prng_range = (prng_range < type_range) ? prng_range + 1 : type_range;
            temp = ext_prng_range * generate_uniform_int<common_type>(gen, 0, range / ext_prng_range);
            res  = temp + (common_type(gen()) - prng_min);
        } while (res >= range || res < temp);
    } else {
        res = common_type(gen()) - prng_min;
    }

    return static_cast<T>(min + res);

    // Note 1:
    // 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
    // those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
    // true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
    // on any platform (see https://en.cppreference.com/w/cpp/types/integer)
    //
    // This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
    // This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.

    // Note 2:
    // 'ext_prng_range' has a ternary purely to silence a false compiler warning from about division by zero due to
    // 'prng_range + 1' overflowing into '0' when 'prng_range' is equal to 'type_range'. Falling into this runtime
    // branch requires 'prng_range < range <= type_range' making such situation impossible, here we simply clamp the
    // value to 'type_range' so it doesn't overflow and trip the compiler when analyzing constexpr for potential UB.
    
    // Note 3:
    // 'static_cast<T>()' in return is functionally useless, but prevents some false positive warnings on MSVC
}

// 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
// those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
// true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
// on any platform (see https://en.cppreference.com/w/cpp/types/integer)
//
// This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
// This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.
template <class T = int, require_integral<T> = true>
struct UniformIntDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    };

    constexpr UniformIntDistribution() = default;
    constexpr UniformIntDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformIntDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return generate_uniform_int<result_type>(gen, this->pars.min, this->pars.max);
    }

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return generate_uniform_int<result_type>(gen, p.min, p.max);
    } // for std-compatibility

    constexpr void                      reset() const noexcept {} // nothing to reset, provided for std-compatibility
    [[nodiscard]] constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void                      params(const param_type& p) noexcept { *this = UniformIntDistribution(p); }
    [[nodiscard]] constexpr result_type a() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type b() const noexcept { return this->pars.max; }
    [[nodiscard]] constexpr result_type min() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type max() const noexcept { return this->pars.max; }

    constexpr bool operator==(const UniformIntDistribution& other) noexcept {
        return this->a() == other.a() && this->b() == other.b();
    }
    constexpr bool operator!=(const UniformIntDistribution& other) noexcept { return !(*this == other); }

private:
    param_type pars{};
};

// --- Uniform real distribution ---
// ---------------------------------

// Can't really make things work without making some reasonable assumptions about the size of primitive types,
// this should be satisfied for the vast majority of platforms. Esoteric architectures can manually adapt the
// algorithm if that is necessary.
static_assert(std::numeric_limits<double>::digits == 53, "Platform not supported, 'float' is expected to be 32-bit.");
static_assert(std::numeric_limits<float>::digits == 24, "Platform not supported, 'double' is expected to be 64-bit.");

template <class T>
constexpr int bit_width(T value) noexcept {
    int width = 0;
    while (value >>= 1) ++width;
    return width;
}

// Constexpr reimplementation of 'std::generate_canonical<>()'
template <class T, class Gen>
constexpr T generate_canonical_generic(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr int float_bits = std::numeric_limits<float_type>::digits;
    // always produce enough bits of randomness for the whole mantissa

    constexpr generated_type prng_max = Gen::max();
    constexpr generated_type prng_min = Gen::min();

    constexpr generated_type prng_range = prng_max - prng_min;
    constexpr generated_type type_range = std::numeric_limits<generated_type>::max();

    constexpr int prng_bits = (prng_range < type_range) ? bit_width(prng_range + 1) : 1 + bit_width(prng_range);
    // how many full bits of randomness PRNG produces on each invocation, prng_bits == floor(log2(prng_range + 1)),
    // ternary handles the case that would overflow when (prng_range == type_range)

    constexpr int invocations_needed = [&]() {
        int count = 0;
        for (int generated_bits = 0; generated_bits < float_bits; generated_bits += prng_bits) ++count;
        return count;
    }();
    // GCC and MSVC use runtime conversion to floating point and std::ceil() & std::log() to obtain
    // this value, in MSVC for example we have something like this:
    //    > invocations_needed = std::ceil( float_type(float_bits) / std::log2( float_type(prng_range) + 1 ) )
    // which is not constexpr due to math functions, we can do a similar thing much easier by just counting bits
    // generated per each invocation. This returns the same thing for any sane PRNG, except since it only counts
    // "full bits" esoteric ranges such as [1, 3] which technically have 1.5 bits of randomness will be counted
    // as 1 bit of randomness, thus overestimating the invocations a little. In practice this makes 0 difference
    // since its only matters for exceedingly small 'prng_range' and such PRNGs simply don't exist in nature, and
    // even if they are theoretically used they will simply use a few more invocation to produce a proper result

    constexpr float_type prng_float_max   = static_cast<float_type>(prng_max);
    constexpr float_type prng_float_min   = static_cast<float_type>(prng_min);
    constexpr float_type prng_float_range = (prng_float_max - prng_float_min) + float_type(1);

    float_type res    = float_type(0);
    float_type factor = float_type(1);

    for (int i = 0; i < invocations_needed; ++i) {
        res += (static_cast<float_type>(gen()) - static_cast<float_type>(prng_min)) * factor;
        factor *= prng_float_range;
    }
    res /= factor;
    // same algorithm is used by 'std::generate_canonical<>' in GCC/clang as of 2025, MSVC used to do the same
    // before the P0952R2 overhaul (see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0952r2.html)

    if (res >= float_type(1)) res = float_type(1) - std::numeric_limits<float_type>::epsilon() / float_type(2);
    // GCC patch the fixes occasional generation of '1's, has non-zero effect on performance

    return res;
}

// Wrapper that adds special case optimizations for `_generate_canonical_generic<>()'
template <class T, class Gen>
constexpr T generate_canonical(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr generated_type prng_min = Gen::min();
    constexpr generated_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr generated_type prng_range          = prng_max - prng_min;
    constexpr generated_type type_range          = std::numeric_limits<generated_type>::max();
    constexpr bool           prng_is_bit_uniform = (prng_range == type_range);

    constexpr int exponent_bits_64 = 11;
    constexpr int exponent_bits_32 = 8;

    constexpr double mantissa_hex_64 = 0x1.0p-53;  // == 2^-53, corresponds to 53 significant bits of double
    constexpr float  mantissa_hex_32 = 0x1.0p-24f; // == 2^-24, corresponds to 24 significant bits of float

    constexpr double pow2_minus_64 = 0x1.0p-64; // == 2^-64
    constexpr double pow2_minus_32 = 0x1.0p-32; // == 2^-32

    // Note 1: Note hexadecimal float literals, 'p' separates hex-base from the exponent
    // Note 2: Floats have 'mantissa_size + 1' significant bits due to having a sign bit
    // Note 3: All of the methods below produce [0, 1) range

    // Bit-uniform PRNGs can be simply bitmasked & shifted to obtain mantissa
    // 64-bit float, 64-bit uniform PRNG
    // => multiplication algorithm, see [https://prng.di.unimi.it/]
    if constexpr (prng_is_bit_uniform && sizeof(float_type) == 8 && sizeof(generated_type) == 8) {
        return (gen() >> exponent_bits_64) * mantissa_hex_64;
    }
    // 64-bit float, 32-bit uniform PRNG
    // => "low-high" algorithm, see [https://www.doornik.com/research/randomdouble.pdf]
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 8 && sizeof(generated_type) == 4) {
        return (gen() * pow2_minus_64) + (gen() * pow2_minus_32);
    }
    // 32-bit float, 64-bit uniform PRNG
    // => discard bits + multiplication algorithm
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 8) {
        return (static_cast<std::uint32_t>(gen()) >> exponent_bits_32) * mantissa_hex_32;
    }
    // 32-bit float, 32-bit uniform PRNG
    // => multiplication algorithm tweaked for 32-bit
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 4) {
        return (gen() >> exponent_bits_32) * mantissa_hex_32;
    }
    // Generic case, no particular optimizations can be made
    else {
        return generate_canonical_generic<T>(gen);
    }
}

template <class T = double, require_float<T> = true>
struct UniformRealDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    } pars{};

    constexpr UniformRealDistribution() = default;
    constexpr UniformRealDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformRealDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr result_type operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return this->pars.min + generate_canonical<result_type>(gen) * (this->pars.max - this->pars.min);
    }

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return p.min + generate_canonical<result_type>(gen) * (p.max - p.min);
    } // for std-compatibility

    constexpr void        reset() const noexcept {} // nothing to reset, provided for std-API compatibility
    constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void        params(const param_type& p) noexcept { *this = UniformRealDistribution(p); }
    constexpr result_type a() const noexcept { return this->pars.min; }
    constexpr result_type b() const noexcept { return this->pars.max; }
    constexpr result_type min() const noexcept { return this->pars.min; }
    constexpr result_type max() const noexcept { return this->pars.max; }

    constexpr bool operator==(const UniformRealDistribution& other) noexcept {
        return this->a() == other.a() && this->b() == other.b();
    }
    constexpr bool operator!=(const UniformRealDistribution& other) noexcept { return !(*this == other); }
};

// --- Normal distribution ---
// ---------------------------

template <class T = double, require_float<T> = true>
struct NormalDistribution {
    using result_type = T;

    struct param_type {
        result_type mean   = 0;
        result_type stddev = 1;
    } pars{};

private:
    // Marsaglia Polar algorithm generates values in pairs so we need to cache the 2nd one
    result_type saved           = 0;
    bool        saved_available = false;

    // Implementation of Marsaglia Polar method for N(0, 1) based on libstdc++,
    // the algorithm is exactly the same, except we use a faster uniform distribution
    // ('generate_canonical()' that was implemented earlier)
    //
    // Note 1:
    // Even if 'generate_canonical()' produced [0, 1] range instead of [0, 1),
    // this would not be an issue since Marsaglia Polar is a rejection method and does
    // not care about the inclusion of upper-boundaries, they get rejected by 'r2 > T(1)' check
    //
    // Note 2:
    // As far as normal distributions go we have 3 options:
    //    - Box-Muller
    //    - Marsaglia Polar
    //    - Ziggurat
    // Box-Muller performance is similar to Marsaglia Polar, but it has issues working with [0, 1]
    // 'generate_canonical()'. Ziggurat is usually ~50% faster, but involver several KB of lookup tables
    // and a MUCH more cumbersome and difficult to generalize implementation. Most (in fact, all I've seen so far)
    // ziggurat implementations found online are absolutely atrocious. There is a very interesting and well-made
    // paper by Christopher McFarland (2015, see https://pmc.ncbi.nlm.nih.gov/articles/PMC4812161/ for pdf) than
    // proposes several significant improvements, but it has even more lookup tables (~12 KB in total) and an even
    // harder implementation. For the sake of robustness we will stick to Polar method for now.
    //
    // Note 3:
    // Not 'constexpr' due to the <cmath> nonsense, can't do anything about it, will be fixed with C++23.
    //
    template <class Gen>
    result_type generate_standard_normal(Gen& gen) noexcept {
        if (this->saved_available) {
            this->saved_available = false;
            return this->saved;
        }

        result_type x, y, r2;

        do {
            x  = T(2) * generate_canonical<result_type>(gen) - T(1);
            y  = T(2) * generate_canonical<result_type>(gen) - T(1);
            r2 = x * x + y * y;
        } while (r2 > T(1) || r2 == T(0));

        const result_type mult = std::sqrt(-2 * std::log(r2) / r2);

        this->saved_available = true;
        this->saved           = x * mult;

        return y * mult;
    }

public:
    constexpr NormalDistribution() = default;
    constexpr NormalDistribution(T mean, T stddev) noexcept : pars({mean, stddev}) { assert(stddev >= T(0)); }
    constexpr NormalDistribution(const param_type& p) noexcept : pars(p) { assert(p.stddev >= T(0)); }

    template <class Gen>
    result_type operator()(Gen& gen) noexcept {
        return this->generate_standard_normal(gen) * this->pars.stddev + this->pars.mean;
    }

    template <class Gen>
    result_type operator()(Gen& gen, const param_type& params) noexcept {
        assert(params.stddev >= T(0));
        return this->generate_standard_normal(gen) * params.stddev + params.mean;
    }

    constexpr void reset() noexcept {
        this->saved           = 0;
        this->saved_available = false;
    }
    [[nodiscard]] constexpr param_type  param() const noexcept { return this->pars; }
    constexpr void                      param(const param_type& p) noexcept { *this = NormalDistribution(p); }
    [[nodiscard]] constexpr result_type mean() const noexcept { return this->pars.mean; }
    [[nodiscard]] constexpr result_type stddev() const noexcept { return this->pars.stddev; }
    [[nodiscard]] constexpr result_type min() const noexcept { return std::numeric_limits<result_type>::lowest(); }
    [[nodiscard]] constexpr result_type max() const noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr bool operator==(const NormalDistribution& other) noexcept {
        return this->mean() == other.mean() && this->stddev() == other.stddev() &&
               this->saved_available == other.saved_available && this->saved == other.saved;
    }
    constexpr bool operator!=(const NormalDistribution& other) noexcept { return !(*this == other); }
};

// --- Approximate normal distribution ---
// ---------------------------------------

// Extremely fast, but noticeably imprecise normal distribution, can be very useful for fuzzing & gamedev

template <class T, require_uint<T> = true>
[[nodiscard]] constexpr int popcount(T x) noexcept {
    constexpr auto bitmask_1 = T(0x5555555555555555UL);
    constexpr auto bitmask_2 = T(0x3333333333333333UL);
    constexpr auto bitmask_3 = T(0x0F0F0F0F0F0F0F0FUL);

    constexpr auto bitmask_16 = T(0x00FF00FF00FF00FFUL);
    constexpr auto bitmask_32 = T(0x0000FFFF0000FFFFUL);
    constexpr auto bitmask_64 = T(0x00000000FFFFFFFFUL);

    x = (x & bitmask_1) + ((x >> 1) & bitmask_1);
    x = (x & bitmask_2) + ((x >> 2) & bitmask_2);
    x = (x & bitmask_3) + ((x >> 4) & bitmask_3);

    if constexpr (sizeof(T) > 1) x = (x & bitmask_16) + ((x >> 8) & bitmask_16);
    if constexpr (sizeof(T) > 2) x = (x & bitmask_32) + ((x >> 16) & bitmask_32);
    if constexpr (sizeof(T) > 4) x = (x & bitmask_64) + ((x >> 32) & bitmask_64);

    return x; // GCC seem to be smart enough to replace this with a built-in
} // C++20 adds a proper 'std::popcount()'

// Quick approximation of normal distribution based on this excellent reddit thread:
// https://www.reddit.com/r/algorithms/comments/yyz59u/fast_approximate_gaussian_generator/
//
// Lack of <cmath> functions also allows us to 'constexpr' everything

template <class T>
[[nodiscard]] constexpr T approx_standard_normal_from_u32_pair(std::uint32_t major, std::uint32_t minor) noexcept {
    constexpr T delta = T(1) / T(4294967296); // (1 / 2^32)

    T x = T(popcount(major)); // random binomially distributed integer 0 to 32
    x += minor * delta;       // linearly fill the gaps between integers
    x -= T(16.5);             // re-center around 0 (the mean should be 16+0.5)
    x *= T(0.3535534);        // scale to ~1 standard deviation
    return x;

    // 'x' now has a mean of 0, stddev very close to 1, and lies strictly in [-5.833631, 5.833631] range,
    // there are exactly 33 * 2^32 possible outputs which is slightly more than 37 bits of entropy,
    // the distribution is approximated via 33 equally spaced intervals each of which is further subdivided
    // into 2^32 parts. As a result we have a very fast, but noticeably inaccurate approximation, not suitable
    // for research, but might prove very useful in fuzzing / gamedev where quality is not that important.
}

template <class T>
[[nodiscard]] constexpr T approx_standard_normal_from_u64(std::uint64_t rng) noexcept {
    return approx_standard_normal_from_u32_pair<T>(static_cast<std::uint32_t>(rng >> 32),
                                                   static_cast<std::uint32_t>(rng));
}

template <class T, class Gen>
constexpr T approx_standard_normal(Gen& gen) noexcept {
    // Ensure PRNG is bit-uniform
    using generated_type = typename Gen::result_type;

    static_assert(Gen::min() == 0);
    static_assert(Gen::max() == std::numeric_limits<generated_type>::max());

    // Forward PRNG to a fast approximation
    if constexpr (sizeof(generated_type) == 8) {
        return approx_standard_normal_from_u64<T>(gen());
    } else if constexpr (sizeof(generated_type) == 4) {
        return approx_standard_normal_from_u32_pair<T>(gen(), gen());
    } else {
        static_assert(always_false_v<T>, "ApproxNormalDistribution<> only supports bit-uniform 32/64-bit PRNGs.");
        // we could use a slower fallback for esoteric PRNGs, but I think it's better to explicitly state when "fast
        // approximate" is not available, esoteric PRNGs are already handled by a regular NormalDistribution
    }
}

template <class T = double, require_float<T> = true>
struct ApproxNormalDistribution {
    using result_type = T;

    struct param_type {
        result_type mean   = 0;
        result_type stddev = 1;
    } pars{};

    constexpr ApproxNormalDistribution() = default;
    constexpr ApproxNormalDistribution(T mean, T stddev) noexcept : pars({mean, stddev}) { assert(stddev >= T(0)); }
    constexpr ApproxNormalDistribution(const param_type& p) noexcept : pars(p) { assert(p.stddev >= T(0)); }

    template <class Gen>
    constexpr result_type operator()(Gen& gen) const noexcept {
        return approx_standard_normal<result_type>(gen) * this->pars.stddev + this->pars.mean;
    }

    template <class Gen>
    constexpr result_type operator()(Gen& gen, const param_type& params) const noexcept {
        assert(params.stddev >= T(0));
        return approx_standard_normal<result_type>(gen) * params.stddev + params.mean;
    }

    constexpr void                     reset() const noexcept {} // nothing to reset, provided for std-API compatibility
    [[nodiscard]] constexpr param_type param() const noexcept { return this->pars; }
    constexpr void                     param(const param_type& p) noexcept { *this = NormalDistribution(p); }
    [[nodiscard]] constexpr result_type mean() const noexcept { return this->pars.mean; }
    [[nodiscard]] constexpr result_type stddev() const noexcept { return this->pars.stddev; }
    [[nodiscard]] constexpr result_type min() const noexcept { return std::numeric_limits<result_type>::lowest(); }
    [[nodiscard]] constexpr result_type max() const noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr bool operator==(const ApproxNormalDistribution& other) noexcept {
        return this->mean() == other.mean() && this->stddev() == other.stddev();
    }
    constexpr bool operator!=(const ApproxNormalDistribution& other) noexcept { return !(*this == other); }
};

// =========================
// --- Convenient random ---
// =========================

// Note 1:
// Despite the intuitive judgement, creating new distribution objects on each call doesn't introduce
// any meaningful overhead. There is however a bit of additional overhead due to 'thread_local' branch
// caused by the thread local PRNG. It is still much faster that 'rand()' or regular <random> usage, but
// if user want to have the bleeding edge performance they should use distributions manually.

// Note 2:
// No '[[nodiscard]]' since random functions inherently can't be pure due to advancing the generator state.
// Discarding return values while not very sensible, can still be done for the sake of advancing state.

// Note 3:
// "Convenient" random uses a thread-local PRNG lazily initialized with entropy, this is a very sane default for
// most cases. If explicit seeding is needed it can be achieved with 'thread_local_prng().seed(...)'.

// --- Global PRNG ---
// -------------------

using PRNG = generators::Xoshiro256PP;

PRNG& thread_local_prng() {
    // no '[[nodiscard]]' as it can be used for a side effect of initializing PRNG
    // no 'noexcept' because entropy source can allocate & fail
    thread_local PRNG prng(entropy_seq());
    return prng;
}

// --- Distribution functions ---
// ------------------------------

// Generic variate, other functions are implemented in terms of this one
template <class Dist>
auto variate(Dist&& dist) -> typename std::decay_t<Dist>::result_type {
    return dist(thread_local_prng());
}

// Integer U[min, max]
template <class T, require_integral<T> = true>
T uniform(T min, T max) {
    return variate(UniformIntDistribution<T>{min, max});
}

// Boolean U[0, 1]
template <class T, require_integral<T> = true, require_bool<T> = true>
T uniform() {
    return variate(UniformIntDistribution<std::uint8_t>{0, 1});
}

// Float U[min, max)
template <class T, require_float<T> = true>
T uniform(T min, T max) {
    return variate(UniformRealDistribution<T>{min, max});
}

// Float U[0, 1)
template <class T, require_float<T> = true>
T uniform() {
    return variate(UniformRealDistribution<T>{0, 1});
}

// Float N(mean, stddev)
template <class T, require_float<T> = true>
T normal(T mean, T stddev) {
    return variate(NormalDistribution<T>{mean, stddev});
    // slower due to discarding state, but that is unavoidable in this API
}

// Float N(0, 1)
template <class T, require_float<T> = true>
T normal() {
    thread_local NormalDistribution<T> dist{};
    return variate(dist);
    // this version can use distribution state properly due to the constant parameters
}

// Choose random element from a list
template <class T>
T choose(std::initializer_list<T> list) {
    return list.begin()[uniform<std::size_t>(0, list.size() - 1)];
}

template <class Container>
auto choose(const Container& list) {
    return list.at(uniform<std::size_t>(0, list.size() - 1));
}

// --- Typed shortcuts ---
// -----------------------

using Uint = unsigned int;

// clang-format off
inline    int uniform_int   (   int min,    int max) { return uniform<   int>(min, max); }
inline   Uint uniform_uint  (  Uint min,   Uint max) { return uniform<  Uint>(min, max); }
inline   bool uniform_bool  (                      ) { return uniform<  bool>(        ); }
inline  float uniform_float ( float min,  float max) { return uniform< float>(min, max); }
inline double uniform_double(double min, double max) { return uniform<double>(min, max); }
inline  float uniform_float (                      ) { return uniform< float>(        ); }
inline double uniform_double(                      ) { return uniform<double>(        ); }

inline  float normal_float ( float mean,  float stddev) { return  normal< float>(mean, stddev); }
inline double normal_double(double mean, double stddev) { return  normal<double>(mean, stddev); }
inline  float normal_float (                          ) { return  normal< float>(            ); }
inline double normal_double(                          ) { return  normal<double>(            ); }
// clang-format on

int    uniform_int(int min, int max);
Uint   uniform_uint(Uint min, Uint max);
bool   uniform_bool();
float  uniform_float(float min, float max);
double uniform_double(double min, double max);
float  uniform_float();
double uniform_double();

float  normal_float(float mean, float stddev);
double normal_double(double mean, double stddev);
float  normal_float();
double normal_double();

} // namespace utl::random::impl

// ______________________ PUBLIC API ______________________

namespace utl::random {

namespace generators = impl::generators;

using impl::entropy_seq;
using impl::entropy;

using impl::UniformIntDistribution;
using impl::UniformRealDistribution;
using impl::NormalDistribution;
using impl::ApproxNormalDistribution;
using impl::generate_canonical;

using impl::PRNG;
using impl::thread_local_prng;

using impl::choose;
using impl::variate;
using impl::uniform;
using impl::normal;

using impl::Uint;
using impl::uniform_int;
using impl::uniform_uint;
using impl::uniform_bool;
using impl::uniform_float;
using impl::uniform_double;
using impl::normal_float;
using impl::normal_double;

using impl::choose;

} // namespace utl::random

#endif
#endif // module utl::random





// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::shell
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_shell.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SHELL)
#ifndef UTLHEADERGUARD_SHELL
#define UTLHEADERGUARD_SHELL

#define UTL_SHELL_VERSION_MAJOR 1
#define UTL_SHELL_VERSION_MINOR 0
#define UTL_SHELL_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <cstdlib>     // system()
#include <filesystem>  // fs::remove(), fs::path, fs::exists(), fs::temp_directory_path()
#include <fstream>     // ofstream, ifstream
#include <stdexcept>   // runtime_error
#include <string>      // string, size_t
#include <string_view> // string_view
#include <thread>      // thread::id, this_thread::get_id()

// ____________________ DEVELOPER DOCS ____________________

// RAII handles for temporary file creation, 'std::system()' wrapper to execute shell commands.
//
// Running commands while capturing both stdout & stderr is surprisingly difficult to do in standard
// C++, piping output to temporary files and then reading them seems to be the most portable way since
// piping works the same way in the vast majority of shells including Windows 'batch'.
//
// API tries to be robust, but at the end of the day we're largely at the whim of the system when it comes
// to filesystem races and shell execution, not much that we could do without hooking to system APIs directly.

// ____________________ IMPLEMENTATION ____________________

namespace utl::shell::impl {

// ==================================
// --- Random filename generation ---
// ==================================

// To generate random filenames we need to generate random characters

// For our use case entropy source has 3 requirements:
//    1. It should be different on each run
//    2. It should be different on each thread
//    3. It should be thread-safe
// Good statistical quality isn't particularly important.
std::uint64_t entropy() {
    const std::uint64_t time_entropy   = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::uint64_t thread_entropy = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return time_entropy ^ thread_entropy;
};

// We want to avoid modifying the state of 'std::rand()', so we use a fast & small thread-local PRNG
constexpr std::uint64_t splitmix64(std::uint64_t state) noexcept {
    std::uint64_t result = (state += 0x9E3779B97f4A7C15);
    result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

// Since char distribution quality isn't particularly important, we can avoid
// including the whole <random> and just use a biased remainder formula
inline char random_char() {
    thread_local std::uint64_t state = entropy();

    state = splitmix64(state);

    constexpr char min_char = 'a';
    constexpr char max_char = 'z';

    return static_cast<char>(min_char + state % std::uint64_t(max_char - min_char + 1));
}

// In the end we have a pretty fast general-purpose random string function
inline std::string random_ascii_string(std::size_t length = 20) {
    std::string result(length, '0');
    for (std::size_t i = 0; i < result.size(); ++i) result[i] = random_char();
    return result;
}

// =======================
// --- Temporary files ---
// =======================

// RAII handle to a temporary file, based on the filesystem and
// naming rather than a proper file handle due to following reasons:
//
//    1. Some temp. file uses (such as command piping) require file to be closed and then reopened again,
//       which means we can't have a persistent file handle without sacrificing a major use case
//
//    2. Before C++23 there is no portable way to open file in exclusive mode,
//       which means we will have possible filesystem race regardless of API
//
struct TemporaryHandle {
private:
    std::filesystem::path filepath;
    std::string           string;
    // makes sense to cache the path string, considering that it is immutable and frequently needed

    explicit TemporaryHandle(std::filesystem::path&& filepath)
        : filepath(std::move(filepath)), string(this->filepath.string()) {

        if (!std::ofstream(this->path())) // creates the file
            throw std::runtime_error("TemporaryHandle(): Could not create {" + this->str() + "}.");
    }

public:
    TemporaryHandle()                       = delete;
    TemporaryHandle(const TemporaryHandle&) = delete;
    TemporaryHandle(TemporaryHandle&&)      = default;

    // --- Construction ---
    // --------------------

    static TemporaryHandle create(std::filesystem::path path) {
        if (std::filesystem::exists(path))
            throw std::runtime_error("TemporaryHandle::create(): File {" + path.string() + "} already exists.");
        return TemporaryHandle(std::move(path));
    }

    static TemporaryHandle create() {
        const std::filesystem::path directory = std::filesystem::temp_directory_path();

        const auto random_path = [&] { return directory / std::filesystem::path(random_ascii_string()); };

        // Try generating random file names until unique one is found, effectively always happens
        // on the first attempt since the probability of a name collision is (1/25)^20 ~= 1e-28
        constexpr std::size_t max_attempts = 50;

        for (std::size_t i = 0; i < max_attempts; ++i) {
            std::filesystem::path path = random_path();

            if (std::filesystem::exists(path)) continue;

            return TemporaryHandle(std::move(path));
        }

        throw std::runtime_error("TemporaryHandle::create(): Could not create a unique filename.");
    }

    static TemporaryHandle overwrite(std::filesystem::path path) { return TemporaryHandle(std::move(path)); }

    static TemporaryHandle overwrite() {
        auto random_path = std::filesystem::temp_directory_path() / std::filesystem::path(random_ascii_string());
        return TemporaryHandle(std::move(random_path));
    }

    // --- Utils ---
    // -------------

    std::ifstream ifstream(std::ios::openmode mode = std::ios::in) const {
        std::ifstream file(this->path(), mode); // 'ifstream' always adds 'std::ios::in'
        if (!file) throw std::runtime_error("TemporaryHandle::ifstream() Could not open {" + this->str() + "}.");
        return file;
    }

    std::ofstream ofstream(std::ios::openmode mode = std::ios::out) const {
        std::ofstream file(this->path(), mode); // 'ofstream' always adds 'std::ios::out'
        if (!file) throw std::runtime_error("TemporaryHandle::ofstream() Could not open {" + this->str() + "}.");
        return file;
    }

    const std::filesystem::path& path() const noexcept { return this->filepath; }
    const std::string&           str() const noexcept { return this->string; }

    // --- Creation ---
    // ----------------

    ~TemporaryHandle() {
        if (!this->filepath.empty()) std::filesystem::remove(this->filepath);
    }
};

// ======================
// --- Shell commands ---
// ======================

// This seems the to be the fastest way of reading a text file
// into 'std::string' without invoking OS-specific methods
[[nodiscard]] inline std::string read_file_to_string(const std::string& path) {
    std::ifstream file(path, std::ios::ate); // open file and immediately seek to the end
    if (!file.good()) throw std::runtime_error("read_file_to_string(): Could not open file {" + path + ".");

    const auto file_size = file.tellg(); // returns cursor pos, which is the end of file
    file.seekg(std::ios::beg);           // seek to the beginning
    std::string chars(file_size, 0);     // allocate string of appropriate size
    file.read(chars.data(), file_size);  // read into the string
    return chars;
}

struct CommandResult {
    int         status; // aka error code
    std::string out;
    std::string err;
};

// A function to run shell command & capture it's status, stdout and stderr.
//

// Note 1:
// Creating temporary files doesn't seem to be ideal, but I'd yet to find
// a way to pipe BOTH stdout and stderr directly into the program without
// relying on platform-specific API like Unix forks and Windows processes
//
// Note 2:
// Usage of std::system() is often discouraged due to security reasons,
// but it doesn't seem there is a portable way to do better (aka going
// back to previous note about platform-specific APIs)
//
inline CommandResult run_command(std::string_view command) {
    const auto stdout_handle = TemporaryHandle::create();
    const auto stderr_handle = TemporaryHandle::create();
    
    constexpr std::string_view stdout_pipe_prefix = " >";
    constexpr std::string_view stderr_pipe_prefix = " 2>";

    // Run command while piping out/err to temporary files
    std::string pipe_command;
    pipe_command.reserve(command.size() + stdout_pipe_prefix.size() + stdout_handle.str().size() +
                         stderr_handle.str().size() + stderr_pipe_prefix.size() + 4);
    pipe_command += command;
    pipe_command += stdout_pipe_prefix;
    pipe_command += '"';
    pipe_command += stdout_handle.str();
    pipe_command += '"';
    pipe_command += stderr_pipe_prefix;
    pipe_command += '"';
    pipe_command += stderr_handle.str();
    pipe_command += '"';

    const int status = std::system(pipe_command.c_str());
    
    // Extract out/err from files
    std::string out = read_file_to_string(stdout_handle.str());
    std::string err = read_file_to_string(stderr_handle.str());
    
    // Remove possible LF/CRLF added by file piping at the end
    if (!out.empty() && out.back() == '\n') out.resize(out.size() - 1); // LF
    if (!out.empty() && out.back() == '\r') out.resize(out.size() - 1); // CR
    if (!err.empty() && err.back() == '\n') err.resize(err.size() - 1); // LF
    if (!err.empty() && err.back() == '\r') err.resize(err.size() - 1); // CR
    
    return {status, std::move(out), std::move(err)};
}

} // namespace utl::shell::impl

// ______________________ PUBLIC API ______________________

namespace utl::shell {

using impl::random_ascii_string;

using impl::TemporaryHandle;

using impl::CommandResult;
using impl::run_command;

} // namespace utl::shell

#endif
#endif // module utl::shell






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::sleep
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_sleep.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SLEEP)
#ifndef UTLHEADERGUARD_SLEEP
#define UTLHEADERGUARD_SLEEP

#define UTL_SLEEP_VERSION_MAJOR 1
#define UTL_SLEEP_VERSION_MINOR 0
#define UTL_SLEEP_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <chrono>  // steady_clock, duration<>, milli
#include <cmath>   // sqrt()
#include <cstdint> // uint64_t
#include <thread>  // this_thread::sleep_for()

// ____________________ DEVELOPER DOCS ____________________

// Various implementations of sleep, useful for sub-millisecond precision delays.
//
// The interesting part is the hybrid sleep implemented based on the idea of this blogpost:
//    https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
// the blogpost itself contains a mostly suitable implementation, but we can noticeably
// improve it in terms of <chrono> compatibility and general robustness. The main idea
// is quite simple, explanation can be found in the comments.

// ____________________ IMPLEMENTATION ____________________

namespace utl::sleep::impl {

// ======================
// --- Spinlock sleep ---
// ======================

using clock = std::chrono::steady_clock;

template <class Rep, class Period>
void spinlock(std::chrono::duration<Rep, Period> duration) {
    const auto start_timepoint = clock::now();

    volatile std::uint64_t i = 0;

    while (clock::now() - start_timepoint < duration) ++i;

    // volatile 'i' prevents standard-compliant compilers from optimizing away the loop,
    // 'std::uint64_t' is large enough to never overflow and even if it hypothetically would,
    // it still wouldn't be an issue (unlike signed overflow, which would be UB)
}

// ====================
// --- System sleep ---
// ====================

template <class Rep, class Period>
void system(std::chrono::duration<Rep, Period> duration) {
    std::this_thread::sleep_for(duration);
}

// ====================
// --- Hybrid sleep ---
// ====================

// The idea is to loop a short system-sleep, measure actual time elapsed,
// and update our estimate for system-sleep duration error on the fly using
// Welford's algorithm, once remaining duration gets small enough relative to
// expected error we swith to a much more precise spinlock-sleep.
//
// This allows us to be almost as precise as a pure spinlock, but instead of
// taking 100% CPU time we only take a few percent.

template <class Rep, class Period>
void hybrid(std::chrono::duration<Rep, Period> duration) {
    using ms = std::chrono::duration<double, std::milli>; // float duration is easier to work with in our context

    constexpr ms short_sleep_duration = ms(1);

    constexpr double stddev_above_mean = 1;
    // how many standard deviations above the error mean should our estimate be,
    // larger value means a more pessimistic estimate

    thread_local ms            err_estimate = ms(5e-3);     // estimates should be per-thread,
    thread_local ms            err_mean     = err_estimate; // having these 'static' would introduce
    thread_local ms            err_m2       = ms(0);        // a race condition in a concurrent scenario
    thread_local std::uint64_t count        = 1;

    ms remaining_duration = duration;

    while (remaining_duration > err_estimate) {
        // Measure actual system-sleep time
        const auto start = clock::now();
        system(short_sleep_duration);
        const auto end = clock::now();

        const ms observed = end - start;
        remaining_duration -= observed;

        ++count;

        // Update error estimate with Welford's algorithm
        const ms delta = observed - err_mean;
        err_mean += delta / count;
        err_m2 += delta.count() * (observed - err_mean); // intermediate values 'm2' reduce numerical instability
        const ms variance = ms(std::sqrt(err_m2.count() / (count - 1)));

        err_estimate = err_mean + stddev_above_mean * variance;
    }

    spinlock(remaining_duration);
}

} // namespace utl::sleep::impl

// ______________________ PUBLIC API ______________________

namespace utl::sleep {

using impl::system;
using impl::spinlock;
using impl::hybrid;

} // namespace utl::sleep

#endif
#endif // module utl::sleep






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::stre
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_stre.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRE)
#ifndef UTLHEADERGUARD_STRE
#define UTLHEADERGUARD_STRE

#define UTL_STRE_VERSION_MAJOR 1
#define UTL_STRE_VERSION_MINOR 0
#define UTL_STRE_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <algorithm>   // transform()
#include <cctype>      // tolower(), toupper()
#include <stdexcept>   // invalid_argument
#include <string>      // string, size_t
#include <string_view> // string_view
#include <vector>      // vector<>

// ____________________ DEVELOPER DOCS ____________________

// String utils. Nothing fancy, basic stuff, however there is a lot of really bad implementations
// posted online, which is why I'd rather put an effort to get them right once and be done with it.
//
// Functions that can reuse the storage of an 'r-value' argument take 'std::string' by value,
// otherwise we taking 'std::string_view' is the most sensible way.

// ____________________ IMPLEMENTATION ____________________

namespace utl::stre::impl {

// ================
// --- Trimming ---
// ================

[[nodiscard]] inline std::string trim_left(std::string str, char trimmed_char = ' ') {
    str.erase(0, str.find_first_not_of(trimmed_char));
    return str;
}

[[nodiscard]] inline std::string trim_right(std::string str, char trimmed_char = ' ') {
    str.erase(str.find_last_not_of(trimmed_char) + 1);
    return str;
}

[[nodiscard]] inline std::string trim(std::string str, char trimmed_char = ' ') {
    return trim_right(trim_left(std::move(str), trimmed_char), trimmed_char);
}

// ===============
// --- Padding ---
// ===============

[[nodiscard]] inline std::string pad_left(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - str.size(), padding_char);
        res += str;
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res += str;
        res.append(length - str.size(), padding_char);
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        const std::size_t left_pad_size = (length - str.size()) / 2;
        res.append(left_pad_size, padding_char);
        res += str;
        const std::size_t right_pad_size = length - str.size() - left_pad_size;
        res.append(right_pad_size, padding_char);
        return res;
        // we try to pad evenly on both sides, but one of the pads (the right one to be exact)
        // may be a character longer than the other if the length difference is odd
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 12) {
    const std::string number_str = std::to_string(number);

    if (length <= number_str.size()) return number_str;

    std::string res;
    res.reserve(length);
    res.append(length - number_str.size(), '0');
    res += number_str;
    return res;
    // we do this instead of using 'std::ostringstream' with 'std::setfill('0')' + 'std::setw()'
    // so we don't need streams as a dependency, plus it is faster that way.
}

// ========================
// --- Case conversions ---
// ========================

[[nodiscard]] inline std::string to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
    // note that 'std::tolower()', 'std::toupper()' can only apply to unsigned chars, calling it on signed char
    // is UB. Implementation above was directly taken from https://en.cppreference.com/w/cpp/string/byte/tolower
}

[[nodiscard]] inline std::string to_upper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
    return str;
}

// ========================
// --- Substring checks ---
// ========================

// Note:
// C++20 adds 'std::basic_string<T>::starts_with()', 'std::basic_string<T>::ends_with()',
// 'std::basic_string<T>::contains()', making these functions pointless in a new standard.

[[nodiscard]] inline bool starts_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(0, substr.size(), substr) == 0;
}

[[nodiscard]] inline bool ends_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(str.size() - substr.size(), substr.size(), substr) == 0;
}

[[nodiscard]] inline bool contains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string_view::npos;
}

// ==========================
// --- Token manipulation ---
// ==========================

[[nodiscard]] inline std::string replace_all_occurrences(std::string str, std::string_view from, std::string_view to) {
    std::size_t i = 0;
    while ((i = str.find(from, i)) != std::string::npos) { // locate substring to replace
        str.replace(i, from.size(), to);                   // replace
        i += to.size();                                    // step over the replaced region
    }
    // Note: Not stepping over the replaced regions causes self-similar replacements
    // like "123" -> "123123" to fall into an infinite loop, we don't want that.

    return str;
}

// Note:
// Most "split by delimiter" implementations found online seem to be horrifically inefficient
// with unnecessary copying/erasure/intermediate tokens, stringstreams and etc.
//
// We can just scan through the string view once, while keeping track of the last segment between
// two delimiters, no unnecessary work, the only place where we do a copy is during emplacement into
// the vector where it's unavoidable
[[nodiscard]] inline std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter,
                                                                 bool keep_empty_tokens = false) {
    if (delimiter.empty()) return {std::string(str)};
    // handle empty delimiter explicitly so we can't fall into an infinite loop

    std::vector<std::string> tokens;
    std::size_t              cursor        = 0;
    std::size_t              segment_start = cursor;

    while ((cursor = str.find(delimiter, cursor)) != std::string_view::npos) {
        if (keep_empty_tokens || segment_start != cursor)
            tokens.emplace_back(str.substr(segment_start, cursor - segment_start));
        // don't emplace empty tokens in case of leading/trailing/repeated delimiter
        cursor += delimiter.size();
        segment_start = cursor;
    }

    if (keep_empty_tokens || segment_start != str.size()) tokens.emplace_back(str.substr(segment_start));
    // 'cursor' is now at 'npos', so we compare to the size instead

    return tokens;
}

// ===================
// --- Other utils ---
// ===================

[[nodiscard]] inline std::string repeat_char(char ch, std::size_t repeats) { return std::string(repeats, ch); }

[[nodiscard]] inline std::string repeat_string(std::string_view str, std::size_t repeats) {
    std::string res;
    res.reserve(str.size() * repeats);
    while (repeats--) res += str;
    return res;
}

// Mostly useful to print strings with special chars in console and look at their contents.
[[nodiscard]] inline std::string escape_control_chars(std::string_view str) {
    std::string res;
    res.reserve(str.size()); // not necessarily correct, but it's a good first guess

    for (const char c : str) {
        // Control characters with dedicated escape sequences get escaped with those sequences
        if (c == '\a') res += "\\a";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else if (c == '\v') res += "\\v";
        // Other non-printable chars get replaced with their codes
        else if (!std::isprint(static_cast<unsigned char>(c))) {
            res += '\\';
            res += std::to_string(static_cast<int>(c));
        }
        // Printable chars are appended as is.
        else
            res += c;
    }
    // Note: This could be implemented faster using the 'utl::json' method of handling escapes with buffering and
    // a lookup table, however I don't see much practical reason to complicate this implementation like that.

    return res;
}

[[nodiscard]] inline std::size_t index_of_difference(std::string_view str_1, std::string_view str_2) {
    using namespace std::string_literals;
    if (str_1.size() != str_2.size())
        throw std::invalid_argument("String {"s + std::string(str_1) + "} of size "s + std::to_string(str_1.size()) +
                                    " and {"s + std::string(str_2) + "} of size "s + std::to_string(str_2.size()) +
                                    " do not have a meaningful index of difference due to incompatible sizes."s);
    for (std::size_t i = 0; i < str_1.size(); ++i)
        if (str_1[i] != str_2[i]) return i;
    return str_1.size();
}

} // namespace utl::stre::impl

// ______________________ PUBLIC API ______________________

namespace utl::stre {

using impl::trim_left;
using impl::trim_right;
using impl::trim;

using impl::pad_left;
using impl::pad_right;
using impl::pad;

using impl::pad_with_leading_zeroes;

using impl::to_lower;
using impl::to_upper;

using impl::starts_with;
using impl::ends_with;
using impl::contains;

using impl::replace_all_occurrences;
using impl::split_by_delimiter;

using impl::repeat_char;
using impl::repeat_string;

using impl::escape_control_chars;
using impl::index_of_difference;

} // namespace utl::stre

#endif
#endif // module utl::stre






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






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::table
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_table.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TABLE)
#ifndef UTLHEADERGUARD_TABLE
#define UTLHEADERGUARD_TABLE

#define UTL_TABLE_VERSION_MAJOR 0 // [!] module awaiting a rewrite
#define UTL_TABLE_VERSION_MINOR 1
#define UTL_TABLE_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <iomanip>          // resetiosflags(), setw()
#include <ios>              // streamsize, ios_base::fmtflags, ios
#include <iostream>         // cout
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <string>           // string
#include <type_traits>      // is_arithmetic_v<>, is_same_v<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Functions used to build and render simple ASCII table in console.
//
// Tries to be simple and minimize boilerplate, exposes a LaTeX-like API.
// In fact this is for a reason - these tables can be formatted for a quick LaTeX export
// by enabling a 'set_latex_mode(true)'.
//
// As of now the implementation is short but quite frankly ugly, it feels like with some thought
// it could be generalized much better to support multiple styles and perform faster, however there
// is ~0 need for this to be fast since it's mean for human-readable tables and not massive data export.
// Adding more styles while nice also doesn't seem like an important thing as of now so the old implementation
// is left to be as is.

// ____________________ IMPLEMENTATION ____________________

namespace utl::table::impl {

// =====================
// --- Column Format ---
// =====================

using uint      = std::streamsize;
using ios_flags = std::ios_base::fmtflags;

struct ColumnFormat {
    ios_flags flags;
    uint      precision;
};

struct Column {
    uint         width;
    ColumnFormat col_format;
};

// --- Predefined Formats ---
// --------------------------

constexpr ColumnFormat NONE = {std::ios::showpoint, 6};

constexpr ColumnFormat FIXED(uint decimals = 3) { return {std::ios::fixed, decimals}; }
constexpr ColumnFormat DEFAULT(uint decimals = 6) { return {std::ios::showpoint, decimals}; }
constexpr ColumnFormat SCIENTIFIC(uint decimals = 3) { return {std::ios::scientific, decimals}; }

constexpr ColumnFormat BOOL = {std::ios::boolalpha, 3};

// --- Internal Table State ---
// ----------------------------

inline std::vector<Column> columns;
inline std::size_t         current_column = 0;
inline std::ostream*       output_stream  = &std::cout;
inline bool                latex_mode     = false;

// ===================
// --- Table Setup ---
// ===================

inline void create(std::initializer_list<uint>&& widths) {
    columns.resize(widths.size());
    for (std::size_t i = 0; i < columns.size(); ++i) {
        columns[i].width      = widths.begin()[i];
        columns[i].col_format = DEFAULT();
    }
}

inline void set_formats(std::initializer_list<ColumnFormat>&& formats) {
    for (std::size_t i = 0; i < columns.size(); ++i) columns[i].col_format = formats.begin()[i];
}

inline void set_ostream(std::ostream& new_ostream) { output_stream = &new_ostream; }

inline void set_latex_mode(bool toggle) { latex_mode = toggle; }

// =======================
// --- Table Rendering ---
// =======================

// We want to only apply additional typesetting to "actual mathematical numbers", not bools & chars
template <class T>
constexpr bool _is_arithmetic_number_v =
    std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>;

[[nodiscard]] inline std::string _trim_left(const std::string& str, char trimmed_char) {
    std::string res = str;
    res.erase(0, res.find_first_not_of(trimmed_char));
    return res;
}

// Function that adds some LaTeX decorators to appropriate types
template <class T>
void _append_decorated_value(std::ostream& os, const T& value) {
    using V = std::decay_t<T>;

    if (!latex_mode) {
        os << value;
        return;
    }

    if constexpr (_is_arithmetic_number_v<V>) {
        // In order to respect format flags of the table, we have to copy fmt into a stringstream
        // and use IT to stringify a number, simple 'std::to_string()' won't do it here
        std::ostringstream ss;
        ss.copyfmt(os);
        ss.width(0); // cancel out 'std::setw()' that was copied with all the other flags
        ss << value;
        std::string number_string = ss.str();

        // Convert scientific form number to a LaTeX-friendly form,
        // for example, "1.3e-15" becomes "1.3 \cdot 10^{-15}"
        const std::size_t e_index = number_string.find('e');
        if (e_index != std::string::npos) {
            const std::string mantissa = number_string.substr(0, e_index - 1);
            const char        sign     = number_string.at(e_index + 1);
            const std::string exponent = number_string.substr(e_index + 2);

            const bool mantissa_is_one =
                mantissa == "1" || mantissa == "1." || mantissa == "1.0" || mantissa == "1.00" || mantissa == "1.000";
            // dirty, simple, a sensible person would figure this out with math a lookup tables

            number_string.clear();
            if (!mantissa_is_one) { // powers of 10 don't need the fractional part
                number_string += mantissa;
                number_string += " \\cdot ";
            }
            number_string += "10^{";
            if (sign == '-') number_string += sign;
            const std::string trimmed_exponent = _trim_left(exponent, '0');
            number_string += trimmed_exponent.empty() ? "0" : trimmed_exponent; // prevent stuff like '10^{}'
            number_string += '}';
        }

        // Typeset numbers as formulas
        os << "$" + number_string + "$";
        // we append it as a single string so ostream 'setw()' doesn't mess up alignment
    } else os << value;
}

inline void cell(){};

template <class T, class... Types>
void cell(const T& value, const Types&... other_values) {
    const auto left_delim  = latex_mode ? "" : "|";
    const auto delim       = latex_mode ? " & " : "|";
    const auto right_delim = latex_mode ? " \\\\\n" : "|\n";

    const std::string left_cline      = (current_column == 0) ? left_delim : "";
    const std::string right_cline     = (current_column == columns.size() - 1) ? right_delim : delim;
    const ios_flags   format          = columns[current_column].col_format.flags;
    const uint        float_precision = columns[current_column].col_format.precision;

    // Save old stream state
    std::ios old_state(nullptr);
    old_state.copyfmt(*output_stream);

    // Set table formatting
    (*output_stream) << std::resetiosflags((*output_stream).flags());
    (*output_stream).flags(format);
    (*output_stream).precision(float_precision);

    // Print
    (*output_stream) << left_cline << std::setw(columns[current_column].width);
    _append_decorated_value(*output_stream, value);
    (*output_stream) << right_cline;

    // Return old stream state
    (*output_stream).copyfmt(old_state);

    // Advance column counter
    current_column = (current_column == columns.size() - 1) ? 0 : current_column + 1;

    cell(other_values...);
}

inline void hline() {
    if (latex_mode) {
        (*output_stream) << "\\hline\n";
    } else {
        (*output_stream) << "|";
        for (const auto& col : columns)
            (*output_stream) << std::string(static_cast<std::size_t>(col.width), '-') << "|";
        (*output_stream) << "\n";
    }
}

} // namespace utl::table::impl

// ______________________ PUBLIC API ______________________

namespace utl::table {

using impl::uint;

using impl::create;

using impl::set_formats;
using impl::set_ostream;
using impl::set_latex_mode;

using impl::cell;
using impl::hline;

using impl::NONE;
using impl::FIXED;
using impl::DEFAULT;
using impl::SCIENTIFIC;
using impl::BOOL;

} // namespace utl::table

#endif
#endif // module utl::table






// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::time
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_time.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TIME)
#ifndef UTLHEADERGUARD_TIME
#define UTLHEADERGUARD_TIME

#define UTL_TIME_VERSION_MAJOR 1
#define UTL_TIME_VERSION_MINOR 0
#define UTL_TIME_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <chrono>      // steady_clock, system_clock, duration_cast<>(), duration<>, time_point<>
#include <cstddef>     // size_t
#include <ctime>       // strftime, mktime
#include <stdexcept>   // runtime_error
#include <string>      // string, to_string()
#include <type_traits> // common_type_t<>

// ____________________ DEVELOPER DOCS ____________________

// Thin wrapper around <chrono> and <ctime> to make common things easier, initially
// started as 'utl::timer' which was a convenient global-state timer that dealt in doubles.
// After some time and a good read through <chrono> documentation it was deprecated in favor
// of a this 'utl::time' module, the rewrite got rid of any global state and added better type
// safety by properly using <chrono> type system.
//
// The reason we can do things so conveniently is because chrono 'duration' and 'time_moment'
// are capable of wrapping around any arithmetic-like type, including floating-point types which
// are properly supported. This is a bit cumbersome to do "natively" which is why it is rarely
// seen in the wild, but with a few simple wrappers things become quite concise.

// ____________________ IMPLEMENTATION ____________________

namespace utl::time::impl {

// ======================
// --- <chrono> utils ---
// ======================

struct SplitDuration {
    std::chrono::hours        hours;
    std::chrono::minutes      min;
    std::chrono::seconds      sec;
    std::chrono::milliseconds ms;
    std::chrono::microseconds us;
    std::chrono::nanoseconds  ns;

    constexpr static std::size_t size = 6; // number of time units, avoids magic constants everywhere

    using common_rep = std::common_type_t<decltype(hours)::rep, decltype(min)::rep, decltype(sec)::rep,
                                          decltype(ms)::rep, decltype(us)::rep, decltype(ns)::rep>;
    // standard doesn't specify common representation type, usually it's 'std::int64_t'

    std::array<common_rep, SplitDuration::size> count() {
        return {this->hours.count(), this->min.count(), this->sec.count(),
                this->ms.count(),    this->us.count(),  this->ns.count()};
    }
};

template <class Rep, class Period>
[[nodiscard]] SplitDuration unit_split(std::chrono::duration<Rep, Period> val) {
    // for some reason 'duration_cast<>()' is not 'noexcept'
    const auto hours = std::chrono::duration_cast<std::chrono::hours>(val);
    const auto min   = std::chrono::duration_cast<std::chrono::minutes>(val - hours);
    const auto sec   = std::chrono::duration_cast<std::chrono::seconds>(val - hours - min);
    const auto ms    = std::chrono::duration_cast<std::chrono::milliseconds>(val - hours - min - sec);
    const auto us    = std::chrono::duration_cast<std::chrono::microseconds>(val - hours - min - sec - ms);
    const auto ns    = std::chrono::duration_cast<std::chrono::nanoseconds>(val - hours - min - sec - ms - us);
    return {hours, min, sec, ms, us, ns};
}

template <class Rep, class Period>
[[nodiscard]] std::string to_string(std::chrono::duration<Rep, Period> value, std::size_t relevant_units = 3) {

    // Takes 'unit_count' of the highest relevant units and converts them to string,
    // for example with 'unit_count' equal to '3', we will have:
    //
    // timescale <= hours   =>   show { hours, min, sec }   =>   string "___ hours ___ min ___ sec"
    // timescale <= min     =>   show {   min, sec,  ms }   =>   string "___ min ___ sec ___ ms"
    // timescale <= sec     =>   show {   sec,  ms,  us }   =>   string "___ sec ___ ms ___ us"
    // timescale <= ms      =>   show {    ms,  us,  ns }   =>   string "___ ms ___ us ___ ns"
    // timescale <= us      =>   show {    us,  ns      }   =>   string "___ us ___ ns"
    // timescale <= ns      =>   show {    ns           }   =>   string "___ ns"

    if (relevant_units == 0) return ""; // early escape for a pathological case

    const std::array<SplitDuration::common_rep, SplitDuration::size> counts = unit_split(value).count();
    const std::array<const char*, SplitDuration::size>               names  = {"hours", "min", "sec", "ms", "us", "ns"};

    for (std::size_t unit = 0; unit < counts.size(); ++unit) {
        if (counts[unit]) {
            std::string res;

            const std::size_t last = (unit + relevant_units < counts.size()) ? (unit + relevant_units) : counts.size();
            // don't want to include the whole <algorithm> just for 'std::max()'

            for (std::size_t k = unit; k < last; ++k) {
                res += std::to_string(counts[k]);
                res += ' ';
                res += names[k];
                res += ' ';
            }

            res.resize(res.size() - 1); // remove trailing space at the end

            return res;
        }
    }

    return "0 ns"; // fallback, unlikely to ever be triggered
}

// ===========================
// --- Floating-point time ---
// ===========================

template <class T>
using float_duration = std::chrono::duration<double, typename T::period>;

using ns    = float_duration<std::chrono::nanoseconds>;
using us    = float_duration<std::chrono::microseconds>;
using ms    = float_duration<std::chrono::milliseconds>;
using sec   = float_duration<std::chrono::seconds>;
using min   = float_duration<std::chrono::minutes>;
using hours = float_duration<std::chrono::hours>;

// Note:
// A cool thing about floating-point-represented time is that we don't need 'std::chrono::duration_cast<>()'
// for conversions, float time satisfies 'treat_as_floating_point_v<>' which means implicit conversions between
// duration can happen for any period, in a nutshell instead of this:
//    > std::chrono::duration_cast<time::ms>(std::chrono::nanoseconds(15));
// we can just do this:
//    > time::ms(std::chrono::nanoseconds(15));
// and it's allowed to happen implicitly.

// =================
// --- Stopwatch ---
// =================

template <class Clock = std::chrono::steady_clock>
struct Stopwatch {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;

    Stopwatch() { this->start(); }

    void start() { this->_start = clock::now(); }

    [[nodiscard]] duration elapsed() const { return clock::now() - this->_start; }

    [[nodiscard]] ns    elapsed_ns() const { return this->elapsed(); }
    [[nodiscard]] us    elapsed_us() const { return this->elapsed(); }
    [[nodiscard]] ms    elapsed_ms() const { return this->elapsed(); }
    [[nodiscard]] sec   elapsed_sec() const { return this->elapsed(); }
    [[nodiscard]] min   elapsed_min() const { return this->elapsed(); }
    [[nodiscard]] hours elapsed_hours() const { return this->elapsed(); }
    // <chrono> handles conversion to a floating-point representation when casting duration to the return type

    [[nodiscard]] std::string elapsed_string(std::size_t relevant_units = 3) const {
        return to_string(this->elapsed(), relevant_units);
    }

private:
    time_point _start;
};

// =============
// --- Timer ---
// =============

template <class Clock = std::chrono::steady_clock>
struct Timer {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;

    Timer() = default;

    template <class Rep, class Period>
    explicit Timer(std::chrono::duration<Rep, Period> length) {
        this->start(length);
    }

    template <class Rep, class Period>
    void start(std::chrono::duration<Rep, Period> length) {
        this->_start  = clock::now();
        this->_length = std::chrono::duration_cast<duration>(length);
    }

    void stop() noexcept { *this = Timer{}; }

    [[nodiscard]] duration elapsed() const { return clock::now() - this->_start; }

    [[nodiscard]] ns    elapsed_ns() const { return this->elapsed(); }
    [[nodiscard]] us    elapsed_us() const { return this->elapsed(); }
    [[nodiscard]] ms    elapsed_ms() const { return this->elapsed(); }
    [[nodiscard]] sec   elapsed_sec() const { return this->elapsed(); }
    [[nodiscard]] min   elapsed_min() const { return this->elapsed(); }
    [[nodiscard]] hours elapsed_hours() const { return this->elapsed(); }
    // <chrono> handles conversion to a floating-point representation when casting duration to the return type

    [[nodiscard]] std::string elapsed_string(std::size_t relevant_units = 3) const {
        return to_string(this->elapsed(), relevant_units);
    }

    [[nodiscard]] bool     finished() const { return this->elapsed() >= this->_length; }
    [[nodiscard]] bool     running() const noexcept { return this->_length != duration{}; }
    [[nodiscard]] duration length() const noexcept { return this->_length; }

private:
    time_point _start{};
    duration   _length{};
};

// ======================
// --- Local datetime ---
// ======================

inline std::tm to_localtime(const std::time_t& time) {
    // There are 3 ways of getting localtime in C-stdlib:
    //    1. 'std::localtime()' - isn't thread-safe and will be marked as "deprecated" by MSVC
    //    2. 'localtime_r()'    - isn't a part of C++, it's a part of C11, in reality provided by POSIX
    //    3. 'localtime_s()'    - isn't a part of C++, it's a part of C23, in reality provided by Windows
    //                            with reversed order of arguments
    // Seemingly there is no portable way of getting thread-safe localtime without being screamed at by at least one
    // compiler, however there is a little known trick that uses a side effect of 'std::mktime()' which normalizes its
    // inputs should they "overflow" the allowed range. Unlike 'localtime', 'std::mktime()' is thread-safe and portable,
    // see https://stackoverflow.com/questions/54246983/c-how-to-fix-add-a-time-offset-the-calculation-is-wrong/54248414

    // Create reference time moment at year 2025
    std::tm reference_tm{};
    reference_tm.tm_isdst = -1;  // negative => let the implementation deal with daylight savings
    reference_tm.tm_year  = 125; // counting starts from 1900

    // Get the 'std::time_t' corresponding to the reference time moment
    const std::time_t reference_time = std::mktime(&reference_tm);
    if (reference_time == -1)
        throw std::runtime_error("time::to_localtime(): time moment can't be represented as 'std::time_t'.");

    // Adjusting reference moment by 'time - reference_time' makes it equal to the current time moment,
    // it is now invalid due to seconds overflowing the allowed range
    reference_tm.tm_sec += static_cast<int>(time - reference_time);
    // 'std::time_t' is an arithmetic type, although not defined, this is almost always an
    // integral value holding the number of seconds since Epoch (see cppreference). This is
    // why we can substract them and add into the seconds.

    // Normalize time moment, it is now valid and corresponds to a current local time
    if (std::mktime(&reference_tm) == -1)
        throw std::runtime_error("time::to_localtime(): time moment can't be represented as 'std::time_t'.");

    return reference_tm;
}

[[nodiscard]] inline std::string datetime_string(const char* format = "%Y-%m-%d %H:%M:%S") {
    const auto now    = std::chrono::system_clock::now();
    const auto c_time = std::chrono::system_clock::to_time_t(now);
    const auto c_tm   = to_localtime(c_time);

    std::array<char, 256> buffer;
    if (std::strftime(buffer.data(), buffer.size(), format, &c_tm) == 0)
        throw std::runtime_error("time::datetime_string(): 'format' does not fit into the buffer.");
    return std::string(buffer.data());

    // Note 1: C++20 provides <chrono> with a native way of getting date, before that we have to use <ctime>
    // Note 2: 'std::chrono::system_clock' is unique - its output can be converted into a C-style 'std::time_t'
    // Note 3: This function is thread-safe, we use a quirky implementation of 'localtime()', see notes above
}

} // namespace utl::time::impl

// ______________________ PUBLIC API ______________________

namespace utl::time {

using impl::SplitDuration;

using impl::unit_split;
using impl::to_string;

using impl::float_duration;

using impl::ns;
using impl::us;
using impl::ms;
using impl::sec;
using impl::min;
using impl::hours;

using impl::Stopwatch;
using impl::Timer;

using impl::to_localtime;
using impl::datetime_string;

} // namespace utl::time

#endif
#endif // module utl::time






