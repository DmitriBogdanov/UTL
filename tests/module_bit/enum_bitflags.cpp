#include "tests/common.hpp"

#include "include/UTL/bit.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // uint8_t

// ____________________ IMPLEMENTATION ____________________

enum class IOMode { IN = 1 << 0, OUT = 1 << 1, APP = 1 << 2 };

TEST_CASE("Enum bitflags / Flag creation") {
    constexpr auto flags_1 = bit::Flags{IOMode::OUT, IOMode::APP};
    constexpr auto flags_2 = bit::Flags(IOMode::OUT) | bit::Flags(IOMode::APP);
    constexpr auto flags_3 = bit::Flags(IOMode::OUT) | IOMode::APP;
    constexpr auto flags_4 = bit::Flags(IOMode::OUT).add(IOMode::APP);
    constexpr auto flags_5 = bit::Flags<IOMode>{}.add(IOMode::OUT).add(IOMode::APP);
    
    static_assert(flags_1 == flags_2 && flags_2 == flags_3 && flags_3 == flags_4 && flags_4 == flags_5);
}

TEST_CASE("Enum bitflags / Method chaining") { 
    auto flags = bit::Flags<IOMode>{};
    
    flags.remove(IOMode::APP);
    flags.add(bit::Flags{IOMode::OUT, IOMode::APP});
    
    CHECK(flags.contains(bit::Flags{IOMode::OUT, IOMode::APP}));
    
    CHECK(flags.contains(IOMode::OUT));
    CHECK(flags.contains(IOMode::APP));
    
    flags.remove(IOMode::APP);
    
    CHECK(flags.contains(IOMode::OUT));
    CHECK(!flags.contains(IOMode::APP));
}