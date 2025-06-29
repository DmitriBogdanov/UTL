#include "tests/common.hpp"

#include "include/UTL/random.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Tests that integer distribution doesn't go outside of a given range,
// mostly a sanity check rather than a statistical one

// =============
// --- Utils ---
// =============

template <class T, class Func>
void test_in_every_int_range(Func&& func) {
    constexpr auto type_max = nl<T>::max();
    constexpr auto type_min = nl<T>::min();

    // try range [ -2^N, 2^N ] for all N
    if constexpr (std::is_signed_v<T>) {
        auto min = T(-1);
        auto max = T(1);
        for (; max < type_max / 2; min *= 2, max *= 2) func(min, max);
        func(nl<T>::min(), type_max);
    }
    // try range [ 0,    2^N ] for all N
    {
        auto max = T(1);
        for (; max < type_max / 2; max *= 2) func(T(0), max);
        func(T(0), type_max);
    }
    // try range [ -2^N,   0 ] for all N
    if constexpr (std::is_signed_v<T>) {
        auto min = T(-1);
        for (; min > type_min / 2; min *= 2) func(min, T(0));
        func(type_min, T(0));
    }
}

// =============
// --- Tests ---
// =============

TEST_CASE_TEMPLATE("Uniform int range", T, //
                   char,                   //
                   std::int8_t,            //
                   std::int16_t,           //
                   std::int32_t,           //
                   std::int64_t,           //
                   std::int8_t,            //
                   std::uint16_t,          //
                   std::uint32_t,          //
                   std::uint64_t           //
) {
    random::generators::SplitMix32 gen;

    constexpr std::size_t sample = 120; // large samples are slower

    // Check that values never escape [min, max] range
    test_in_every_int_range<T>([&](T min, T max) {
        const random::UniformIntDistribution dist{min, max};
        for (std::size_t i = 0; i < sample; ++i) {
            const auto value = dist(gen);
            FAST_CHECK(min <= value);
            FAST_CHECK(max >= value);
        }
    });
}