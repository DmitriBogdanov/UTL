#include "tests/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

#include <algorithm> // min(), max()
#include <cstdint>   // std::int64_t

// ____________________ IMPLEMENTATION ____________________

constexpr std::size_t repeats = 1;
// we run tests multiple times to increase the chance of catching race conditions,
// this is usually kept low for CI builds, can be increased locally for better coverage

constexpr std::size_t threads = 7;  // weird number of threads
constexpr std::size_t N       = 37; // prime number to make things never evenly divisible

// Note: Large values of 'N' might cause product to overflow

// --- 'Range' blocking reduce for different binary ops (4) ---
// ------------------------------------------------------------

TEST_CASE("Parallel-reduce (Range) / Blocking sum") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial += vec[i];

        parallel::set_thread_count(threads);
        std::int64_t res_parallel = parallel::blocking_reduce(parallel::Range{vec}, parallel::sum<>{});
        parallel::set_thread_count(0);

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Blocking prod") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial *= vec[i];

        parallel::set_thread_count(threads);
        std::int64_t res_parallel = parallel::blocking_reduce(parallel::Range{vec}, parallel::prod<>{});
        parallel::set_thread_count(0);

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Blocking min") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial = std::min(res_serial, vec[i]);

        parallel::set_thread_count(threads);
        std::int64_t res_parallel = parallel::blocking_reduce(parallel::Range{vec}, parallel::min<>{});
        parallel::set_thread_count(0);

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Blocking max") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial = std::max(res_serial, vec[i]);

        parallel::set_thread_count(threads);
        std::int64_t res_parallel = parallel::blocking_reduce(parallel::Range{vec}, parallel::max<>{});
        parallel::set_thread_count(0);

        REQUIRE(res_parallel == res_serial);
    });
}

// --- 'Range' awaitable reduce for different binary ops (4) ---
// -------------------------------------------------------------

TEST_CASE("Parallel-reduce (Range) / Awaitable sum") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial += vec[i];

        parallel::set_thread_count(threads);
        auto future       = parallel::awaitable_reduce(parallel::Range{vec}, parallel::sum<>{});
        auto res_parallel = future.get();

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Awaitable prod") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial *= vec[i];

        parallel::set_thread_count(threads);
        auto future       = parallel::awaitable_reduce(parallel::Range{vec}, parallel::prod<>{});
        auto res_parallel = future.get();

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Awaitable min") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial = std::min(res_serial, vec[i]);

        parallel::set_thread_count(threads);
        auto future       = parallel::awaitable_reduce(parallel::Range{vec}, parallel::min<>{});
        auto res_parallel = future.get();

        REQUIRE(res_parallel == res_serial);
    });
}

TEST_CASE("Parallel-reduce (Range) / Awaitable max") {
    repeat(repeats, [] {
        std::vector<std::int64_t> vec(N, 0);
        for (std::size_t i = 0; i < N; ++i) vec[i] = i;

        std::int64_t res_serial = vec[0];
        for (std::size_t i = 1; i < N; ++i) res_serial = std::max(res_serial, vec[i]);

        parallel::set_thread_count(threads);
        auto future       = parallel::awaitable_reduce(parallel::Range{vec}, parallel::max<>{});
        auto res_parallel = future.get();

        REQUIRE(res_parallel == res_serial);
    });
}