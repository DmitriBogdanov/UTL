#include "include/UTL/bit.hpp"

using namespace utl;

// Bitflag-suitable enum class
enum class io_mode { in = 1 << 0, out = 1 << 1, app = 1 << 2 };

int main() { 
    
    // Construct from multiple flags
    constexpr auto flags_1 = bit::flags{io_mode::out, io_mode::app};
    
    // Combine flag sets
    constexpr auto flags_2 = bit::flags(io_mode::out) | bit::flags(io_mode::app);
    
    // Combine with individual flags
    constexpr auto flags_3 = bit::flags(io_mode::out) | io_mode::app;
    
    // Combine by chaining '.add()'
    constexpr auto flags_4 = bit::flags<io_mode>{}.add(io_mode::out).add(io_mode::app);
    
    static_assert(flags_1 == flags_2 && flags_2 == flags_3 && flags_3 == flags_4);
}