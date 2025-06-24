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
