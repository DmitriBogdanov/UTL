#include "tests/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

#include <random> // mt19937, uniform_int_distribution<>

// ____________________ IMPLEMENTATION ____________________

constexpr std::size_t repeats = 5;
// we run tests multiple times to increase the chance of catching race conditions,
// this is usually kept low for CI builds, can be increased locally for better coverage

TEST_CASE("Fuzzing / Detached task loss") {
    // Enqueues random number of small tasks with randomly sized thread pool
    // and tests that all tasks finished their work successfully

    std::mt19937                               gen(16);
    std::uniform_int_distribution<std::size_t> thread_count_dist{3, 47};
    std::uniform_int_distribution<std::size_t> vector_size_dist{1, 2237};

    repeat(repeats, [&] {
        parallel::ThreadPool pool(thread_count_dist(gen));

        std::vector<std::size_t> vec(vector_size_dist(gen));
        for (std::size_t i = 0; i < vec.size(); ++i) pool.detached_task([i, &vec] { vec[i] = i * i; });

        pool.wait();

        for (std::size_t i = 0; i < vec.size(); ++i) REQUIRE(vec[i] == i * i);
    });
}

TEST_CASE("Fuzzing / Thread pool creation") {
    // Re-creates thread pool with many random sizes, resizes it several
    // times and checks that pool size is always correctly adjusted

    std::mt19937                               gen(16);
    std::uniform_int_distribution<std::size_t> thread_count_dist{0, 127};

    repeat(repeats, [&] {
        const std::size_t    init_size = thread_count_dist(gen);
        parallel::ThreadPool pool(init_size);
        REQUIRE(pool.get_thread_count() == init_size);

        const std::size_t size_1 = thread_count_dist(gen);
        pool.set_thread_count(size_1);
        REQUIRE(pool.get_thread_count() == size_1);

        const std::size_t size_2 = thread_count_dist(gen);
        pool.set_thread_count(size_2);
        REQUIRE(pool.get_thread_count() == size_2);
    });
}

TEST_CASE("Fuzzing / Modifying thread pool from multiple threads") {
    // Resizes thread pool concurrently from multiple threads, while submitting more tasks at the same time,
    // if thread pool API is really thread-safe, the pool should complete all the submitted tasks and not
    // cause any memory or threading issues, this is a stress-test that would fail most existing thread pools

    std::mt19937                               gen(16);
    std::uniform_int_distribution<std::size_t> thread_count_dist{1, 7};

    constexpr std::size_t modifying_threads = 12;
    constexpr std::size_t tasks_per_thread  = 20;

    using namespace std::chrono_literals;

    repeat(repeats, [&] {
        parallel::ThreadPool pool(4);

        std::atomic<std::size_t> tasks_completed = 0;

        // Spawn multiple threads with 'std::async()' and have them concurrently enqueue
        // tasks into the pool while resizing it from multiple threads at the same time
        std::vector<std::future<void>> futures;

        for (std::size_t i = 0; i < modifying_threads; ++i) {
            const std::size_t pool_threads = thread_count_dist(gen);

            futures.emplace_back(std::async([=, &pool, &tasks_completed] {
                for (std::size_t i = 0; i < tasks_per_thread; ++i)
                    pool.detached_task([&] {
                        std::this_thread::sleep_for(1ms);
                        ++tasks_completed;
                    });

                pool.set_thread_count(pool_threads);
            }));
        }

        for (auto& future : futures) future.wait();

        REQUIRE(tasks_completed == modifying_threads * tasks_per_thread);
    });
}