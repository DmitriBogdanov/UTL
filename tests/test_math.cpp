// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/math.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ DEVELOPER DOCS ____________________

// All tests run a compile time.

// ____________________ IMPLEMENTATION ____________________

// ====================================
// --- Template math function tests ---
// ====================================

TEST_CASE("Template math functions work as expected") {
    // Basic functions (integer)
    static_assert(math::abs(4) == 4);
    static_assert(math::abs(-5) == 5);
    static_assert(math::sign(15) == 1);
    static_assert(math::sign(-4) == -1);
    static_assert(math::sqr(1) == 1);
    static_assert(math::sqr(-7) == 49);
    static_assert(math::cube(1) == 1);
    static_assert(math::cube(-3) == -27);
    static_assert(math::heaviside(1) == 1);
    static_assert(math::heaviside(0) == 0);
    static_assert(math::heaviside(-3) == 0);

    static_assert(math::midpoint(20, 30) == 25);
    static_assert(math::absdiff('a', 'b') == char(1));
    static_assert(math::absdiff('b', 'a') == char(1));
    static_assert(math::absdiff(15u, 18u) == 3u);
    static_assert(math::absdiff(18u, 15u) == 3u);

    // Basic functions (floating point)
    static_assert(math::abs(4.f) == Flt{4.f});
    static_assert(math::abs(-5.f) == Flt{5.f});
    static_assert(math::sign(15.f) == Flt{1.f});
    static_assert(math::sign(-4.f) == Flt{-1.f});
    static_assert(math::sqr(1.f) == Flt{1.f});
    static_assert(math::sqr(-7.f) == Flt{49.f});
    static_assert(math::cube(1.f) == Flt{1.f});
    static_assert(math::cube(-3.f) == Flt{-27.f});
    static_assert(math::heaviside(1.f) == Flt{1.f});
    static_assert(math::heaviside(0.f) == Flt{0.f});
    static_assert(math::heaviside(-3.f) == Flt{0.f});

    static_assert(math::midpoint(20.f, 30.f) == Flt{25.f});
    static_assert(math::absdiff(2.5f, 3.5f) == Flt{1.f});

    // Power functions (integer)
    static_assert(math::pow(7, 2) == 49);
    static_assert(math::pow(2, 5) == 32);
    static_assert(math::pow(-2, 7) == -128);
    static_assert(math::signpow(7) == -1);
    static_assert(math::signpow(-3) == -1);
    static_assert(math::signpow(8) == 1);
    static_assert(math::signpow(-4) == 1);

    // Power functions (floating point)
    static_assert(math::pow(7.f, 2) == Flt{49.f});
    static_assert(math::pow(0.5f, 2) == Flt{0.25f});
    static_assert(math::pow(-2.f, 7) == Flt{-128.f});

    // Index functions
    static_assert(math::kronecker_delta(-7, -7) == 1);
    static_assert(math::kronecker_delta(-7, -8) == 0);
    static_assert(math::levi_civita(3, 4, 4) == 0);
    static_assert(math::levi_civita(3, 4, 5) == 1);
    static_assert(math::levi_civita(5, 4, 3) == -1);

    // Conversions
    static_assert(math::deg_to_rad(0.) == Flt{0.});
    static_assert(math::deg_to_rad(360.) == Flt{math::constants::two_pi});
    static_assert(math::deg_to_rad(17 * 180.) == Flt{17. * math::constants::pi});
    static_assert(math::deg_to_rad(-180.) == Flt{-math::constants::pi});
    static_assert(math::rad_to_deg(0.) == Flt{0.});
    static_assert(math::rad_to_deg(math::constants::two_pi) == Flt{360.});
    static_assert(math::rad_to_deg(17. * math::constants::pi) == Flt{17 * 180.});
    static_assert(math::rad_to_deg(-math::constants::pi) == Flt{-180.});

    // Sequence operations
    static_assert(math::sum(0, 3, [](auto i) { return i; }) == 0 + 1 + 2 + 3);
    static_assert(math::sum(0, 4, [](auto i) { return i * i; }) == 0 + 1 + 4 + 9 + 16);
    static_assert(math::sum(-2, 2, [](auto i) { return i * i; }) == 4 + 1 + 0 + 1 + 4);
    static_assert(math::sum(-2, 2, [](auto i) { return i * i; }) == 4 + 1 + 0 + 1 + 4);
    static_assert(math::sum<std::size_t>(0, std::size_t(4), [](auto i) { return 2 * i; }) == 0 + 2 + 4 + 6 + 8);

    static_assert(math::prod(1, 3, [](auto i) { return i; }) == 1 * 2 * 3);
    static_assert(math::prod(1, 4, [](auto i) { return i * i; }) == 1 * 4 * 9 * 16);
    static_assert(math::prod(-2, 2, [](auto i) { return i * i; }) == 4 * 1 * 0 * 1 * 4);
    static_assert(math::prod(-2, 2, [](auto i) { return i * i; }) == 4 * 1 * 0 * 1 * 4);
    static_assert(math::prod<std::size_t>(0, std::size_t(4), [](auto i) { return 2 * i; }) == 0 * 2 * 4 * 6 * 8);
}