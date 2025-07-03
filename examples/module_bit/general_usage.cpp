#include "include/UTL/bit.hpp"

#include <cstdint>

int main() {
    using namespace utl;
    
    constexpr std::uint8_t x = 19; // 19 ~ 00010011
    
    // Group bit operations
    static_assert(bit::rotl(  x, 6) == 196); // 196 ~ 11000100
    static_assert(bit::rotr(  x, 1) == 137); // 137 ~ 10001001
    static_assert(bit::lshift(x, 6) == 192); // 192 ~ 11000000
    static_assert(bit::rshift(x, 1) ==   9); //   9 ~ 00001001
    
    // Utils
    static_assert(bit::width(x) == 5); // 00010011 has 5 significant bits
    
    static_assert(bit::size_of<std::uint16_t> == 16);
    static_assert(bit::size_of<std::uint32_t> == 32);
    static_assert(bit::size_of<std::uint64_t> == 64);
}