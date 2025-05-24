// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/random.hpp"

// _______________________ INCLUDES _______________________

#include <algorithm>   // PRNG sanity tests
#include <array>       // PRNG sanity tests
#include <cstddef>     // PRNG sanity tests
#include <cstdint>     // PRNG sanity tests
#include <numeric>     // PRNG sanity tests
#include <random>      // PRNG sanity tests
#include <type_traits> // PRNG sanity tests
#include <vector>      // PRNG sanity tests

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ===============================
// --- Basic PRNG sanity tests ---
// ===============================

#define FAST_CHECK(arg_)                                                                                               \
    if (bool(arg_) == false) CHECK(arg_)
// somehow much faster than doing a raw 'CHECK()', I assume this is  
// due to the test framework needing to save successful checks

template <class T, class Func>
void test_in_every_real_range(Func&& func) {
    // try range [ -2^N, 2^N ] for all N
    auto min = T(-1);
    auto max = T(1);
    for (; max < nl<T>::max() / 2; min *= 2, max *= 2) func(min, max);
    // try range [ 0,    2^N ] for all N
    min = T(-1);
    max = T(1);
    for (; max < nl<T>::max() / 2; min *= 2, max *= 2) func(T(0), max);
    // try range [ -2^N,   0 ] for all N
    min = T(-1);
    max = T(1);
    for (; max < nl<T>::max() / 2; min *= 2, max *= 2) func(min, T(0));
}

TEST_CASE_TEMPLATE("Uniform real distribution doesn't go out of range", T, //
                   long double,                                            //
                   double,                                                 //
                   float                                                   //
) {
    random::generators::SplitMix32 gen;

    constexpr std::size_t sample = 120; // large samples are slower

    // Check that values never escape [min, max] range
    test_in_every_real_range<T>([&](T min, T max) {
        const random::UniformRealDistribution dist{min, max};
        for (std::size_t i = 0; i < sample; ++i) {
            const auto value = dist(gen);
            FAST_CHECK(min <= value);
            FAST_CHECK(max >= value);
        }
    });
}

template <class T, class Func>
void test_in_every_int_range(Func&& func) {
    // try range [ -2^N, 2^N ] for all N
    if constexpr (std::is_signed_v<T>) {
        auto min = T(-1);
        auto max = T(1);
        for (; max < nl<T>::max() / 2; min *= 2, max *= 2) func(min, max);
        func(nl<T>::min(), nl<T>::max());
    }
    // try range [ 0,    2^N ] for all N
    if constexpr (true) {
        auto max = T(1);
        for (; max < nl<T>::max() / 2; max *= 2) func(T(0), max);
        func(T(0), nl<T>::max());
    }
    // try range [ -2^N,   0 ] for all N
    if constexpr (std::is_signed_v<T>) {
        auto min = T(-1);
        for (; min > nl<T>::min() / 2; min *= 2) func(min, T(0));
        func(nl<T>::min(), T(0));
    }
}

TEST_CASE_TEMPLATE("Uniform int distribution doesn't go out of range", T, //
                   char,                                                  //
                   std::int8_t,                                           //
                   std::int16_t,                                          //
                   std::int32_t,                                          //
                   std::int64_t,                                          //
                   std::int8_t,                                           //
                   std::uint16_t,                                         //
                   std::uint32_t,                                         //
                   std::uint64_t                                          //
) {
    random::generators::SplitMix32 gen;

    constexpr std::size_t sample = 120; // large samples are slower

    // Check that values never escape [min, max] range
    test_in_every_int_range<T>([&](T min, T max) {
        const random::UniformIntDistribution dist{min, max};
        for (std::size_t i = 0; i < sample; ++i) {
            const auto value = dist(gen);
            // std::cout << "[" << min << " / " << max << "]\n";
            FAST_CHECK(min <= value);
            FAST_CHECK(max >= value);
        }
    });
}

TEST_CASE_TEMPLATE("Uniform int distribution covers whole interval for every type", T, //
                   char,                                                               //
                   std::int8_t,                                                        //
                   std::int16_t,                                                       //
                   std::int32_t,                                                       //
                   std::int64_t,                                                       //
                   std::int8_t,                                                        //
                   std::uint16_t,                                                      //
                   std::uint32_t,                                                      //
                   std::uint64_t                                                       //
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

TEST_CASE_TEMPLATE("Uniform int distribution covers whole interval for every PRNG", Gen, //
                   std::minstd_rand0,                                                    //
                   random::generators::RomuMono16,                                       //
                   random::generators::RomuTrio32,                                       //
                   random::generators::SplitMix32,                                       //
                   random::generators::Xoshiro128PP,                                     //
                   random::generators::RomuDuoJr64,                                      //
                   random::generators::SplitMix64,                                       //
                   random::generators::Xoshiro256PP                                      //
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

double vec_mean(const std::vector<double>& vec) { return std::reduce(vec.begin(), vec.end()) / vec.size(); }

double vec_min(const std::vector<double>& vec) { return *std::min_element(vec.begin(), vec.end()); }

double vec_max(const std::vector<double>& vec) { return *std::max_element(vec.begin(), vec.end()); }

TEST_CASE("Uniform distribution mean/min/max are sensible") {

    constexpr std::size_t N   = 100'000; // number of random vals
    constexpr double      eps = 2e-2;    // epsilon used for double comparison, 1e-2 ~ 1% allowed error
    std::vector<double>   vec(N);

    // Check for all utl::random:: uniform distribution functions
    // that mean/min/max are ~= their expected values
    for (auto& e : vec) e = random::rand_double();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_double(-8., 8.);
    CHECK(vec_mean(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-8.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(8.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_float();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_float(-3., 1.);
    CHECK(vec_mean(vec) == doctest::Approx(-1.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-3.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_int(-90, -80);
    CHECK(vec_mean(vec) == doctest::Approx(-85.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-90.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(-80.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_uint(5, 15);
    CHECK(vec_mean(vec) == doctest::Approx(10.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(5.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(15.0).epsilon(eps));

    for (auto& e : vec) e = static_cast<int>(random::rand_bool());
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::rand_choice({-2, -1, 0, 1, 2});
    CHECK(vec_mean(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-2.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(2.0).epsilon(eps));

    // This is in no way a proper PRNG test, proper testing of PRNGs is usually done through
    // Diehard testing suite:
    // https://en.wikipedia.org/wiki/Diehard_tests
    // and "TestU01" ANSI C library:
    // https://en.wikipedia.org/wiki/TestU01
    // a "good" PRNG would be expected to pass (or at least mostly pass) TestU01 Big Crush,
    // however it is a task for PRNG designers, here we merely implement well known algorithms
    // and check that their implementation wasn't accidentally broken.
}