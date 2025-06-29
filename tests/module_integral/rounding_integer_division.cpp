#include "tests/common.hpp"

#include "include/UTL/integral.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Rounding integer division") {
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