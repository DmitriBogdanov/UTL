#include "tests/common.hpp"

#include "include/UTL/bit.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // uint8_t, uint16_t, uint32_t, uint64_t

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Utils") {
    static_assert(bit::size_of<std::uint16_t> == 16);
    static_assert(bit::size_of<std::uint32_t> == 32);
    static_assert(bit::size_of<std::uint64_t> == 64);

    // Test different values for various integer widths:
    //                  19 ~ 00010011
    //                 -27 ~ 11111111'11100101
    //                   0 ~ 00000000'00000000'00000000'00000000
    // 9223372036854775715 ~ 01111111'11111111'11111111'11111111'11111111'11111111'11111111'10100011
    constexpr std::uint8_t x1 = 19;
    constexpr std::int16_t x2 = -27;
    constexpr std::int32_t x3 = 0;
    constexpr std::int64_t x4 = 9223372036854775715;

    static_assert(bit::width(x1) == 5);    // x1 has 5 significant bits
    static_assert(bit::popcount(x1) == 3); // x1 has 3 set bits

    static_assert(bit::width(x2) == 16);    // x2 has 16 significant bits
    static_assert(bit::popcount(x2) == 13); // x2 has 13 set bits

    static_assert(bit::width(x3) == 0);    // x3 has 0 significant bits
    static_assert(bit::popcount(x3) == 0); // x3 has 0 set bits

    static_assert(bit::width(x4) == 63);    // x4 has 63 significant bits
    static_assert(bit::popcount(x4) == 59); // x4 has 59 set bits
}