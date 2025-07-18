#include "tests/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

constexpr std::size_t repeats = 1;
// we run tests multiple times to increase the chance of catching race conditions,
// this is usually kept low for CI builds, can be increased locally for better coverage

constexpr std::size_t threads = 7;    // weird number of threads
constexpr std::size_t N       = 1367; // prime number to make things never evenly divisible
constexpr int         x       = 17;   // test value

// --- 'IndexRange' overloads (6) ---
// ----------------------------------

TEST_CASE("Parallel-for (IndexRange) / Detached block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::detached_loop(parallel::IndexRange{0, vec.size()}, [&](auto low, auto high) {
            for (auto i = low; i < high; ++i) vec[i] = x;
        });
        parallel::set_thread_count(0);

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Parallel-for (IndexRange) / Detached iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::detached_loop(parallel::IndexRange{0, vec.size()}, [&](auto i) { vec[i] = x; });
        parallel::set_thread_count(0);

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Parallel-for (IndexRange) / Blocking block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::blocking_loop(parallel::IndexRange{0, vec.size()}, [&](auto low, auto high) {
            for (auto i = low; i < high; ++i) vec[i] = x;
        });

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Parallel-for (IndexRange) / Blocking iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::blocking_loop(parallel::IndexRange{0, vec.size()}, [&](auto i) { vec[i] = x; });

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Parallel-for (IndexRange) / Awaitable block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        auto future = parallel::awaitable_loop(parallel::IndexRange{0, vec.size()}, [&](auto low, auto high) {
            for (auto i = low; i < high; ++i) vec[i] = x;
        });
        future.wait();

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Parallel-for (IndexRange) / Awaitable iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        auto future = parallel::awaitable_loop(parallel::IndexRange{0, vec.size()}, [&](auto i) { vec[i] = x; });
        future.wait();

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

// --- 'Container' overloads (3) ---
// ---------------------------------

TEST_CASE("Container parallel-for / Detached block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::detached_loop(vec, [&](auto low, auto high) {
            for (auto it = low; it != high; ++it) *it = x;
        });
        parallel::set_thread_count(0);

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Container-for API / Detached iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::detached_loop(vec, [&](auto it) { *it = x; });
        parallel::set_thread_count(0);

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Container parallel-for / Blocking block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::blocking_loop(vec, [&](auto low, auto high) {
            for (auto it = low; it != high; ++it) *it = x;
        });

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Container-for API / Blocking iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        parallel::blocking_loop(vec, [&](auto it) { *it = x; });

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Container parallel-for / Awaitable block loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        auto future = parallel::awaitable_loop(vec, [&](auto low, auto high) {
            for (auto it = low; it != high; ++it) *it = x;
        });
        future.wait();

        for (const auto& e : vec) REQUIRE(e == x);
    });
}

TEST_CASE("Container-for API / Awaitable iteration loop") {
    repeat(repeats, [] {
        std::vector<int> vec(N, 0);

        parallel::set_thread_count(threads);
        auto future = parallel::awaitable_loop(vec, [&](auto it) { *it = x; });
        future.wait();

        for (const auto& e : vec) REQUIRE(e == x);
    });
}