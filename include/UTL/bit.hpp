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
