// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/integral.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // testing arithmetics on fixed-size types

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________


// ==============================
// --- Integer division tests ---
// ==============================

using namespace integral::literals;

TEST_CASE("Rounding integral division behaves as expected") {
    static_assert(integral::div_ceil(6, 3) == 2);
    static_assert(integral::div_ceil(5, 3) == 2);
    static_assert(integral::div_ceil(4, 3) == 2);
    static_assert(integral::div_ceil(3, 3) == 1);

    static_assert(integral::div_ceil(-6, 3) == -2);
    static_assert(integral::div_ceil(-5, 3) == -1);
    static_assert(integral::div_ceil(-4, 3) == -1);
    static_assert(integral::div_ceil(-3, 3) == -1);

    static_assert(integral::div_ceil(6, -3) == -2);
    static_assert(integral::div_ceil(5, -3) == -1);
    static_assert(integral::div_ceil(4, -3) == -1);
    static_assert(integral::div_ceil(3, -3) == -1);

    static_assert(integral::div_ceil(-6, -3) == 2);
    static_assert(integral::div_ceil(-5, -3) == 2);
    static_assert(integral::div_ceil(-4, -3) == 2);
    static_assert(integral::div_ceil(-3, -3) == 1);

    static_assert(integral::div_floor(6, 3) == 2);
    static_assert(integral::div_floor(5, 3) == 1);
    static_assert(integral::div_floor(4, 3) == 1);
    static_assert(integral::div_floor(3, 3) == 1);

    static_assert(integral::div_floor(-6, 3) == -2);
    static_assert(integral::div_floor(-5, 3) == -2);
    static_assert(integral::div_floor(-4, 3) == -2);
    static_assert(integral::div_floor(-3, 3) == -1);

    static_assert(integral::div_floor(6, -3) == -2);
    static_assert(integral::div_floor(5, -3) == -2);
    static_assert(integral::div_floor(4, -3) == -2);
    static_assert(integral::div_floor(3, -3) == -1);

    static_assert(integral::div_floor(-6, -3) == 2);
    static_assert(integral::div_floor(-5, -3) == 1);
    static_assert(integral::div_floor(-4, -3) == 1);
    static_assert(integral::div_floor(-3, -3) == 1);
}

// ============================
// --- Saturated math tests ---
// ============================

TEST_CASE("Saturated math behaves as expected") {
    static_assert(integral::mul_overflows(64_i8, 64_i8) == true);
    static_assert(integral::mul_overflows(64_i8, 32_i8) == true);
    static_assert(integral::mul_overflows(64_i8, 2_i8) == true);

    static_assert(integral::mul_overflows(nlmin<int>, -2) == true);
    static_assert(integral::mul_overflows(nlmax<int>, -2) == true);
    static_assert(integral::mul_overflows(nlmin<int>, nlmax<int>) == true);

    static_assert(integral::mul_overflows(64_i8, 1_i8) == false);
    static_assert(integral::mul_overflows(4_i8, 7_i8) == false);

    static_assert(integral::mul_overflows(nlmin<int>, 1) == false);
    static_assert(integral::mul_overflows(nlmin<int>, -1) == true);
    static_assert(integral::mul_overflows(nlmax<int>, 1) == false);
    static_assert(integral::mul_overflows(nlmax<int>, -1) == false);

    static_assert(integral::add_sat(2_u8, 3_u8) == 5_u8);
    static_assert(integral::add_sat(7_u32, 14_u32) == 21_u32);
    static_assert(integral::add_sat(13_i32, 9_i32) == 22_i32);

    static_assert(integral::add_sat(255_u8, 1_u8) == 255_u8);
    static_assert(integral::add_sat(nlmax<int>, 2) == nlmax<int>);
    static_assert(integral::add_sat(nlmin<int>, -20) == nlmin<int>);
    static_assert(integral::add_sat(nlmax<std::uint64_t>, 25_u64) == nlmax<std::uint64_t>);

    static_assert(integral::sub_sat(3_u8, 2_u8) == 1_u8);
    static_assert(integral::sub_sat(3_u32, 2_u32) == 1_u8);
    static_assert(integral::sub_sat(13_i32, 9_i32) == 4_i32);

    static_assert(integral::sub_sat(0_u8, 1_u8) == 0_u8);
    static_assert(integral::sub_sat(nlmin<int>, 2) == nlmin<int>);
    static_assert(integral::sub_sat(nlmax<int>, -20) == nlmax<int>);
    static_assert(integral::sub_sat(nlmin<std::uint64_t>, 25_u64) == nlmin<std::uint64_t>);

    static_assert(integral::mul_sat(64_i8, 64_i8) == nlmax<std::int8_t>);
    static_assert(integral::mul_sat(64_i8, 32_i8) == nlmax<std::int8_t>);
    static_assert(integral::mul_sat(64_i8, 2_i8) == nlmax<std::int8_t>);

    static_assert(integral::mul_sat(nlmin<int>, -2) == nlmax<int>);
    static_assert(integral::mul_sat(nlmax<int>, -2) == nlmin<int>);
    static_assert(integral::mul_sat(nlmin<int>, nlmax<int>) == nlmin<int>);

    static_assert(integral::mul_sat(64_i8, 1_i8) == 64_i8);
    static_assert(integral::mul_sat(4_i8, 7_i8) == 28_i8);

    static_assert(integral::mul_sat(nlmin<int>, 1) == nlmin<int>);
    static_assert(integral::mul_sat(nlmin<int>, -1) == nlmax<int>);
    static_assert(integral::mul_sat(nlmax<int>, 1) == nlmax<int>);
    static_assert(integral::mul_sat(nlmax<int>, -1) == nlmin<int> + 1);
    
    static_assert(integral::div_sat(nlmin<int>, -1) == nlmax<int>);
    static_assert(integral::div_sat(nlmax<int>, 1) == nlmax<int>);
}