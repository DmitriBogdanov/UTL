#include "tests/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

constexpr std::size_t repeats = 10;
// we run tests multiple times to increase the chance of catching race conditions
// this is usually kept low for CI build, and can be increased locally for better coverage

TEST_CASE("Threadpool basics / Construct and resize") {
    repeat(repeats, [] {
        parallel::ThreadPool pool;

        println("Resizing to 3");
        pool.set_thread_count(3);
        REQUIRE(pool.get_thread_count() == 3);

        println("Resizing to 7");
        pool.set_thread_count(7);
        REQUIRE(pool.get_thread_count() == 7);

        println("Resizing to 25");
        pool.set_thread_count(25);
        REQUIRE(pool.get_thread_count() == 25);

        println("Resizing to 0");
        pool.set_thread_count(0);
        REQUIRE(pool.get_thread_count() == 0);
    });
}

TEST_CASE("Threadpool basics / Awaitable task with arguments") {
    repeat(repeats, [] {
        parallel::ThreadPool pool(4);

        const auto compute = [](int x) { return x + 32; };
        auto       future  = pool.awaitable_task(compute, 10);
        int        res     = future.get();

        CHECK(res == 42);
    });
}

TEST_CASE("Threadpool basics / Detached tasks with no arguments") {
    repeat(repeats, [] {
        parallel::ThreadPool pool(3);

        std::vector<int> vec(20);
        for (std::size_t i = 0; i < vec.size(); ++i) pool.detached_task([i, &vec] { vec[i] = i * i; });
        pool.wait();

        for (std::size_t i = 0; i < vec.size(); ++i) REQUIRE(vec[i] == i * i);
    });
}

TEST_CASE("Threadpool basics / Recursive awaitable tasks") {
    repeat(repeats, [] {
        std::function<int(int)> fibonacci_serial = [&](int n) {
            if (n < 2) return n;
            const int prev_1 = fibonacci_serial(n - 1);
            const int prev_2 = fibonacci_serial(n - 2);
            return prev_1 + prev_2;
        };

        parallel::ThreadPool pool(3);

        std::function<int(int)> fibonacci_async = [&](int n) {
            println("Fibonacci at ", n);

            if (n < 2) return n;

            auto future_prev_1 = pool.awaitable_task([&] { return fibonacci_async(n - 1); });
            auto future_prev_2 = pool.awaitable_task([&] { return fibonacci_async(n - 2); });

            return future_prev_1.get() + future_prev_2.get();
        };

        const int arg        = 8;
        const int res_serial = fibonacci_serial(arg);
        const int res_async  = fibonacci_async(arg);

        CHECK(res_serial == res_async);
    });
}