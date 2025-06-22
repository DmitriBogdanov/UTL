// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/math.hpp"

// _______________________ INCLUDES _______________________

#include <array>         // type-trait tests
#include <cstddef>       // memory usage estimate tests
#include <cstdint>       // memory usage estimate tests
#include <functional>    // type-trait tests
#include <list>          // memory usage estimate tests
#include <string>        // type-trait tests
#include <string_view>   // type-trait tests
#include <unordered_map> // type-trait tests
#include <unordered_set> // memory usage estimate tests
#include <vector>        // type-trait tests

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ============================
// --- Basic function tests ---
// ============================

TEST_CASE("Basic math functions work as expected") {
    // Standard math functions (integer case should be exact, no floating point conversion involved)
    static_assert(math::abs(4) == 4);
    static_assert(math::abs(-5) == 5);
    static_assert(math::sign(15) == 1);
    static_assert(math::sign(-4) == -1);
    static_assert(math::sqr(1) == 1);
    static_assert(math::sqr(-7) == 49);
    static_assert(math::cube(1) == 1);
    static_assert(math::cube(-3) == -27);
    static_assert(math::pow(7, 2) == 49);
    static_assert(math::pow(2, 5) == 32);
    static_assert(math::pow(-2, 7) == -128);
    static_assert(math::midpoint(20, 30) == 25);
    static_assert(math::kronecker_delta(-7, -7) == 1);
    static_assert(math::kronecker_delta(-7, -8) == 0);
    static_assert(math::signpow(7) == -1);
    static_assert(math::signpow(8) == 1);

    // Standard math functions (floating point case)
    static_assert(math::abs(4.f) == Flt{4.f});
    static_assert(math::abs(-5.f) == Flt{5.f});
    static_assert(math::sign(15.f) == Flt{1.f});
    static_assert(math::sign(-4.f) == Flt{-1.f});
    static_assert(math::sqr(1.f) == Flt{1.f});
    static_assert(math::sqr(-7.f) == Flt{49.f});
    static_assert(math::cube(1.f) == Flt{1.f});
    static_assert(math::cube(-3.f) == Flt{-27.f});
    static_assert(math::pow(7.f, 2) == Flt{49.f});
    static_assert(math::pow(0.5f, 2) == Flt{0.25f});
    static_assert(math::pow(-2.f, 7) == Flt{-128.f});
    static_assert(math::midpoint(20.f, 30.f) == Flt{25.f});

    // Degrees and radians
    static_assert(math::deg_to_rad(0.) == Flt{0.});
    static_assert(math::deg_to_rad(360.) == Flt{math::constants::two_pi});
    static_assert(math::deg_to_rad(17 * 180.) == Flt{17. * math::constants::pi});
    static_assert(math::deg_to_rad(-180.) == Flt{-math::constants::pi});
    static_assert(math::rad_to_deg(0.) == Flt{0.});
    static_assert(math::rad_to_deg(math::constants::two_pi) == Flt{360.});
    static_assert(math::rad_to_deg(17. * math::constants::pi) == Flt{17 * 180.});
    static_assert(math::rad_to_deg(-math::constants::pi) == Flt{-180.});

    // Sum
    static_assert(math::sum(0, 3, [](auto i) { return i; }) == 0 + 1 + 2 + 3);
    static_assert(math::sum(0, 4, [](auto i) { return i * i; }) == 0 + 1 + 4 + 9 + 16);
    static_assert(math::sum(-2, 2, [](auto i) { return i * i; }) == 4 + 1 + 0 + 1 + 4);
    static_assert(math::sum(-2, 2, [](auto i) { return i * i; }) == 4 + 1 + 0 + 1 + 4);
    static_assert(math::sum<std::size_t>(0, std::size_t(4), [](auto i) { return 2 * i; }) == 0 + 2 + 4 + 6 + 8);

    // Meshing
    const auto grid_1 = math::linspace(0., 1., math::Points(3));
    const auto grid_2 = math::linspace(0., 1., math::Intervals(2));
    CHECK(grid_1 == grid_2);
    CHECK(grid_1.size() == 3);
    CHECK(grid_1[0] == doctest::Approx(0.0));
    CHECK(grid_1[1] == doctest::Approx(0.5));
    CHECK(grid_1[2] == doctest::Approx(1.0));

    const auto   f              = [](double x) -> double { return std::pow(x, 6); };
    const double L1             = -2.;
    const double L2             = 4.;
    const auto   integral       = math::integrate_trapezoidal(f, L1, L2, math::Intervals(2000));
    const auto   integral_exact = std::pow(L2, 7) / 7. - std::pow(L1, 7) / 7.;
    CHECK(integral == doctest::Approx(integral_exact).epsilon(1e-4));

    // Misc helpers
    static_assert(math::absdiff('a', 'b') == char(1));
    static_assert(math::absdiff('b', 'a') == char(1));
    static_assert(math::absdiff(15u, 18u) == 3u);
    static_assert(math::absdiff(18u, 15u) == 3u);

    std::vector<int> vec(7);
    CHECK(math::ssize(vec) == 7);

    // Branchless ternary
    static_assert(math::ternary_branchless(true, 17u, 6u) == 17u);
    static_assert(math::ternary_branchless(false, 17u, 6u) == 6u);
    static_assert(math::ternary_bitselect(true, 8, -7) == 8);
    static_assert(math::ternary_bitselect(false, 8, -7) == -7);
    static_assert(math::ternary_bitselect(true, 9) == 9);
    static_assert(math::ternary_bitselect(false, 9) == 0);
}

// =========================
// --- Permutation tests ---
// =========================

TEST_CASE("Permutations work as expected") {
    CHECK(math::sorting_permutation(std::vector<double>{0.5, 2.4, 1.5}) == std::vector<std::size_t>{0, 2, 1});
    CHECK(math::sorting_permutation(std::vector<double>{0.5, 1.5, 2.4}) == std::vector<std::size_t>{0, 1, 2});
    CHECK(math::sorting_permutation(std::vector<double>{2.4, 1.5, 0.5}) == std::vector<std::size_t>{2, 1, 0});

    CHECK(math::is_permutation(std::vector{1, 2, 5, 4, 3, 0}));
    CHECK(math::is_permutation(std::vector{0, 1, 2, 3}));
    CHECK(!math::is_permutation(std::vector{0, 0, 1, 2, 3}));
    CHECK(!math::is_permutation(std::vector{1, 2, 3}));
    CHECK(!math::is_permutation(std::vector{0, 1, 5, 6}));

    std::vector<double> vals = {2., 4., 6., 8.};
    math::apply_permutation(vals, {3, 2, 1, 0});
    CHECK(vals == std::vector<double>{8., 6., 4., 2.});

    std::vector<int>         target_array   = {1, 2, 5, 4, 3};
    std::vector<char>        synced_array_1 = {'1', '2', '5', '4', '3'};
    std::vector<std::string> synced_array_2 = {"1", "2", "5", "4", "3"};
    math::sort_together(target_array, synced_array_1, synced_array_2);
    CHECK(target_array == std::vector<int>{1, 2, 3, 4, 5});
    CHECK(synced_array_1 == std::vector<char>{'1', '2', '3', '4', '5'});
    CHECK(synced_array_2 == std::vector<std::string>{"1", "2", "3", "4", "5"});
}

// =============================
// --- Memory estimate tests ---
// =============================

TEST_CASE("(math::memory_size<T, size_t> == ...) for T = ") {
    constexpr auto byte = math::MemoryUnit::BYTE;

    constexpr std::array<int, 17> arr{};
    static_assert(math::quick_memory_estimate<byte>(arr) == sizeof(arr));

    const std::vector<double> vec(765);
    CHECK(math::quick_memory_estimate<byte>(vec) == 765 * sizeof(double) + sizeof(decltype(vec)));

    const std::string str(45, 'x');
    CHECK(math::quick_memory_estimate<byte>(str) == 45 * sizeof(char) + sizeof(decltype(str)));

    const std::list<std::uint16_t> list = {1, 2, 3, 4, 5, 6, 7};
    CHECK(math::quick_memory_estimate<byte>(list) >= 7 * sizeof(std::uint16_t) + sizeof(decltype(list)));

    const std::unordered_set<std::uint64_t> set = {1, 2, 3, 4, 5};
    CHECK(math::quick_memory_estimate<byte>(set) >= 5 * sizeof(std::uint64_t) + sizeof(decltype(set)));

    const std::unordered_map<std::uint64_t, std::uint64_t> map = {
        {1, 1},
        {2, 2},
        {3, 3}
    };
    CHECK(math::quick_memory_estimate<byte>(map) >= 3 * 2 * sizeof(std::uint64_t) + sizeof(decltype(map)));
}