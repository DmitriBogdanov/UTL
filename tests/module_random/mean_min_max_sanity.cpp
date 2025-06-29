#include "tests/common.hpp"

#include "include/UTL/random.hpp"

// _______________________ INCLUDES _______________________

#include <algorithm> // min_element(), max_element()
#include <numeric>   // reduce()
#include <vector>    // vector<>

// ____________________ IMPLEMENTATION ____________________

// Tests than 'uniform_...()' functions produce "close enough" mean/min/max when averaged
// over a large number of values, mostly a sanity check rather than a statistical one

double vec_mean(const std::vector<double>& vec) { return std::reduce(vec.begin(), vec.end()) / vec.size(); }
double vec_min(const std::vector<double>& vec) { return *std::min_element(vec.begin(), vec.end()); }
double vec_max(const std::vector<double>& vec) { return *std::max_element(vec.begin(), vec.end()); }

TEST_CASE("Mean, min, max sanity") {

    constexpr std::size_t sample = 20'000;
    constexpr double      eps    = 5e-2; // float comparison eps, 5e-2 ~ 5% allowed error
    std::vector<double>   vec(sample);

    random::thread_local_prng().seed(17); // tests should be deterministic

    for (auto& e : vec) e = random::uniform_double();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::uniform_double(-8., 8.);
    CHECK(vec_mean(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-8.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(8.0).epsilon(eps));

    for (auto& e : vec) e = random::uniform_float();
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::uniform_float(-3., 1.);
    CHECK(vec_mean(vec) == doctest::Approx(-1.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-3.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::uniform_int(-90, -80);
    CHECK(vec_mean(vec) == doctest::Approx(-85.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-90.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(-80.0).epsilon(eps));

    for (auto& e : vec) e = random::uniform_uint(5, 15);
    CHECK(vec_mean(vec) == doctest::Approx(10.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(5.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(15.0).epsilon(eps));

    for (auto& e : vec) e = static_cast<int>(random::uniform_bool());
    CHECK(vec_mean(vec) == doctest::Approx(0.5).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(1.0).epsilon(eps));

    for (auto& e : vec) e = random::choose({-2, -1, 0, 1, 2});
    CHECK(vec_mean(vec) == doctest::Approx(0.0).epsilon(eps));
    CHECK(vec_min(vec) == doctest::Approx(-2.0).epsilon(eps));
    CHECK(vec_max(vec) == doctest::Approx(2.0).epsilon(eps));
}