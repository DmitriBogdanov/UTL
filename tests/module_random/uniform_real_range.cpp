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
void test_in_every_real_range(Func&& func) {
    constexpr auto type_max = sizeof(T) <= sizeof(double) ? nl<T>::max() : nl<double>::max();
    // long double '::max()' bloats test runtime a bit too much

    // try range [ -2^N, 2^N ] for all N
    auto min = T(-1);
    auto max = T(1);
    for (; max < type_max / 2; min *= 2, max *= 2) func(min, max);
    // try range [ 0,    2^N ] for all N
    min = T(-1);
    max = T(1);
    for (; max < type_max / 2; min *= 2, max *= 2) func(T(0), max);
    // try range [ -2^N,   0 ] for all N
    min = T(-1);
    max = T(1);
    for (; max < type_max / 2; min *= 2, max *= 2) func(min, T(0));
}

// =============
// --- Tests ---
// =============

TEST_CASE_TEMPLATE("Uniform real range", T, //
                   long double,             //
                   double,                  //
                   float                    //
) {
    random::generators::SplitMix64 gen;

    constexpr std::size_t sample = 120; // large samples are slower

    // Check that values never escape [min, max] range
    test_in_every_real_range<T>([&](T min, T max) {
        const random::UniformRealDistribution dist{min, max};
        for (std::size_t i = 0; i < sample; ++i) {
            const auto value = dist(gen);
            FAST_CHECK(min <= value);
            FAST_CHECK(max > value); // real distributions have range [a, b)
        }
    });
}