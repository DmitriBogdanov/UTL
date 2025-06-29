#include "tests/common.hpp"

#include "include/UTL/bit.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // uint8_t

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Group bit operations") {
    constexpr std::uint8_t x = 19; // 19 ~ 00010011

    // Group bit operations
    static_assert(bit::rotl(x, 6) == 196);   // 196 ~ 11000100
    static_assert(bit::rotr(x, 1) == 137);   // 137 ~ 10001001
    static_assert(bit::lshift(x, 6) == 192); // 192 ~ 11000000
    static_assert(bit::rshift(x, 1) == 9);   //   9 ~ 00001001
}