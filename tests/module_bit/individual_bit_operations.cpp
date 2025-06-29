#include "tests/common.hpp"

#include "include/UTL/bit.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // uint8_t

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Individual bit operations") {
    constexpr std::uint8_t x = 19; // 19 ~ 00010011
    // human-readable notation is big-endian, which means bits are indexed right-to-left

    // Read bits
    static_assert(bit::get(x, 0) == 1);
    static_assert(bit::get(x, 1) == 1);
    static_assert(bit::get(x, 2) == 0);
    static_assert(bit::get(x, 3) == 0);
    static_assert(bit::get(x, 4) == 1);
    static_assert(bit::get(x, 5) == 0);
    static_assert(bit::get(x, 6) == 0);
    static_assert(bit::get(x, 7) == 0);

    // Modify bits
    static_assert(bit::set(x, 2) == 23);   // 23 ~ 00010111
    static_assert(bit::clear(x, 0) == 18); // 18 ~ 00010010
    static_assert(bit::flip(x, 1) == 17);  // 17 ~ 00010001
}