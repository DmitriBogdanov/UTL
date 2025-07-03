#include "include/UTL/bit.hpp"

using namespace utl;

// Bitflag-suitable enum class
enum class IOMode { IN = 1 << 0, OUT = 1 << 1, APP = 1 << 2 };

int main() { 
    
    // Simple yet flexible API, same thing can be accomplished
    // both with classic bit-wise semantics and with built-in methods.
    // Underneath it's just a strongly typed integer so there is no performance impact
    constexpr auto flags_1 = bit::Flags{IOMode::OUT, IOMode::APP};
    constexpr auto flags_2 = bit::Flags(IOMode::OUT) | bit::Flags(IOMode::APP);
    constexpr auto flags_3 = bit::Flags(IOMode::OUT) | IOMode::APP;
    constexpr auto flags_4 = bit::Flags(IOMode::OUT).add(IOMode::APP);
    constexpr auto flags_5 = bit::Flags<IOMode>{}.add(IOMode::OUT).add(IOMode::APP);
    
    static_assert(flags_1 == flags_2 && flags_2 == flags_3 && flags_3 == flags_4 && flags_4 == flags_5);
}