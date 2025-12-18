#include "tests/common.hpp"

#include "include/UTL/bit.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

enum class io_mode { in = 1 << 0, out = 1 << 1, app = 1 << 2 };

TEST_CASE("Enum bitflags / Flag creation") {
    constexpr auto flags_1 = bit::flags{io_mode::out, io_mode::app};
    constexpr auto flags_2 = bit::flags(io_mode::out) | bit::flags(io_mode::app);
    constexpr auto flags_3 = bit::flags(io_mode::out) | io_mode::app;
    constexpr auto flags_4 = bit::flags(io_mode::out).add(io_mode::app);
    constexpr auto flags_5 = bit::flags<io_mode>{}.add(io_mode::out).add(io_mode::app);

    static_assert(flags_1 == flags_2 && flags_2 == flags_3 && flags_3 == flags_4 && flags_4 == flags_5);
}

TEST_CASE("Enum bitflags / Method chaining") {
    auto flags = bit::flags<io_mode>{};

    flags.remove(io_mode::app);
    flags.add(bit::flags{io_mode::out, io_mode::app});

    CHECK(flags.contains(bit::flags{io_mode::out, io_mode::app}));

    CHECK(flags.contains(io_mode::out));
    CHECK(flags.contains(io_mode::app));

    flags.remove(io_mode::app);

    CHECK(flags.contains(io_mode::out));
    CHECK(!flags.contains(io_mode::app));
}