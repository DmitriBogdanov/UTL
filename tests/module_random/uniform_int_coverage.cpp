#include "tests/common.hpp"

#include "include/UTL/random.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Tests that for a small integer range all values will be eventually generated,
// mostly a sanity check rather than a statistical one

TEST_CASE_TEMPLATE("Uniform int coverage / Different types", T, //
                   char,                                        //
                   std::int8_t,                                 //
                   std::int16_t,                                //
                   std::int32_t,                                //
                   std::int64_t,                                //
                   std::int8_t,                                 //
                   std::uint16_t,                               //
                   std::uint32_t,                               //
                   std::uint64_t                                //
) {
    random::generators::SplitMix32 gen;

    constexpr auto min   = T(0);
    constexpr auto max   = T(17);
    constexpr auto count = max - min + 1;

    std::array<bool, count> visited_values{};

    const random::UniformIntDistribution dist{min, max};

    for (std::size_t cursor = 0; cursor < visited_values.size();) {
        visited_values[dist(gen)] = true;
        if (visited_values[cursor]) ++cursor;
    }
}

TEST_CASE_TEMPLATE("Uniform int coverage / Different PRNGs", Gen, //
                   std::minstd_rand0,                             //
                   random::generators::RomuMono16,                //
                   random::generators::RomuTrio32,                //
                   random::generators::SplitMix32,                //
                   random::generators::Xoshiro128PP,              //
                   random::generators::RomuDuoJr64,               //
                   random::generators::SplitMix64,                //
                   random::generators::Xoshiro256PP               //
) {
    Gen gen;

    constexpr int min   = 0;
    constexpr int max   = 17;
    constexpr int count = max - min + 1;

    std::array<bool, count> visited_values{};

    const random::UniformIntDistribution dist{min, max};

    for (std::size_t cursor = 0; cursor < visited_values.size();) {
        visited_values[dist(gen)] = true;
        if (visited_values[cursor]) ++cursor;
    }
}