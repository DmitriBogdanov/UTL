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
