#include "benchmarks/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"
#include "include/UTL/sleep.hpp"

// Libraries to benchmarks against
#include "benchmarks/thirdparty/bshoshany/BS_thread_pool.hpp"
#include "benchmarks/thirdparty/progschj/ThreadPool.h"

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// ==================
// --- Task types ---
// ==================

// [!Important]
//
// We want to emulate workload with busy-waiting, using regular sleep might mess things up
// by allowing other threads to take over while the thread is supposed to be "busy".
//
// Some CPUs are weirdly efficient at running a busy-waiting loop in a hyper-threaded mode,
// leading to speedups closer to the hyper-threaded core count than a physical one.
//
// It is highly recommended to disable on hyper-threading for these benchmarks.
// On Linux it can be toggled with following commands:
//
//    > Query   hyper-threading: 'cat /sys/devices/system/cpu/smt/control'
//    > Enable  hyper-threading: 'echo on | sudo tee /sys/devices/system/cpu/smt/control'
//    > Disable hyper-threading: 'echo off | sudo tee /sys/devices/system/cpu/smt/control'
//
// We could also write a more "realistic" task that wouldn't benefit from hyper-threading as
// much, but busy-waiting loop is more convenient for tweaking benchmark parameters.

constexpr std::size_t task_count = 1000;

struct small_task {
    template <class Pool>
    void execute(std::atomic<std::size_t>& tasks_done, Pool&) {
        constexpr unsigned int duration_min = 0;
        constexpr unsigned int duration_max = 100;

        sleep::spinlock(std::chrono::microseconds(random::uniform_uint(duration_min, duration_max)));

        ++tasks_done;
    }
};

struct large_task {
    template <class Pool>
    void execute(std::atomic<std::size_t>& tasks_done, Pool&) {
        constexpr unsigned int duration_min = 1000;
        constexpr unsigned int duration_max = 3000;

        sleep::spinlock(std::chrono::microseconds(random::uniform_uint(duration_min, duration_max)));

        ++tasks_done;
    }
};

struct shallow_recursive_task {
    template <class Pool>
    void execute(std::atomic<std::size_t>& tasks_done, Pool& pool) {
        constexpr unsigned int duration_min     = 0;
        constexpr unsigned int duration_max     = 100;
        constexpr double       recursion_chance = 0.7; // < 1.0

        sleep::spinlock(std::chrono::microseconds(random::uniform_uint(duration_min, duration_max)));

        if (random::uniform_double() < recursion_chance) {
            shallow_recursive_task subtask;

            auto future = pool.awaitable_task([&] { subtask.execute(tasks_done, pool); });
            future.get();
        }

        ++tasks_done;
    }
};

struct deep_recursive_task {
    template <class Pool>
    void execute(std::atomic<std::size_t>& tasks_done, Pool& pool) {
        constexpr unsigned int duration_min     = 0;
        constexpr unsigned int duration_max     = 100;
        constexpr double       recursion_chance = 0.49; // < 0.5

        sleep::spinlock(std::chrono::microseconds(random::uniform_uint(duration_min, duration_max)));

        if (random::uniform_double() < recursion_chance) {
            shallow_recursive_task subtask;

            auto future_1 = pool.awaitable_task([&] { subtask.execute(tasks_done, pool); });
            auto future_2 = pool.awaitable_task([&] { subtask.execute(tasks_done, pool); });
            future_1.get();
            future_2.get();
        }

        ++tasks_done;
    }
};

// =========================
// --- Thread pool types ---
// =========================

// Here we simply wrap different tasking mechanisms into a uniform API to make benchmarking easier

struct serial_executor {
    template <class = void>
    struct future_type {
        void get() {}
    }; // dummy future to make serial code fit the template of async code without additional overhead

    serial_executor(std::size_t) {} // dummy constructor for the same purpose

    template <class F, class... Args>
    future_type<> awaitable_task(F&& f, Args&&... args) {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return future_type<>{};
    }
};

struct std_async_executor {
    template <class = void>
    using future_type = std::future<void>;

    std_async_executor(std::size_t) {}

    template <class F, class... Args>
    future_type<> awaitable_task(F&& f, Args&&... args) {
        return std::async(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

struct bs_thread_pool_executor {
    template <class = void>
    using future_type = std::future<void>;

    BS::thread_pool<> pool;

    bs_thread_pool_executor(std::size_t count) : pool(count) {}

    template <class F, class... Args>
    future_type<> awaitable_task(F&& f, Args&&... args) {
        auto closure = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return this->pool.submit_task(std::move(closure));
    }
};

struct progschj_thread_pool_executor {
    template <class = void>
    using future_type = std::future<void>;

    ThreadPool pool;

    progschj_thread_pool_executor(std::size_t count) : pool(count) {}

    template <class F, class... Args>
    future_type<> awaitable_task(F&& f, Args&&... args) {
        return this->pool.enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

// =================
// --- Benchmark ---
// =================

template <class Pool, class Task>
void benchmark_thread_pool(const char* name) {
    Pool pool(parallel::hardware_concurrency());
    Task task;

    using future_type = typename Pool::template future_type<>;

    benchmark(name, [&] {
        std::atomic<std::size_t> tasks_done = 0;

        std::vector<future_type> futures;
        futures.reserve(task_count);

        for (std::size_t i = 0; i < task_count; ++i)
            futures.emplace_back(pool.awaitable_task([&] { task.execute(tasks_done, pool); }));

        for (auto& future : futures) future.get();

        DO_NOT_OPTIMIZE_AWAY(tasks_done);
    });
}

#define BENCHMARK_THREAD_POOL(pool_, task_) benchmark_thread_pool<pool_, task_>(#pool_)

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    bench.timeUnit(1ms, "ms").minEpochTime(1s).maxEpochTime(2s).relative(true); // global options

    // clang-format off
    bench.title("Small non-recursive tasks");
    benchmark_thread_pool<serial_executor              , small_task>("Serial"              );
    benchmark_thread_pool<std_async_executor           , small_task>("std::async()"        );
    benchmark_thread_pool<parallel::thread_pool        , small_task>("parallel::ThreadPool");
    benchmark_thread_pool<bs_thread_pool_executor      , small_task>("BS::thread_pool"     );
    benchmark_thread_pool<progschj_thread_pool_executor, small_task>("progschj/ThreadPool" );
    
    bench.title("Large non-recursive tasks");
    benchmark_thread_pool<serial_executor              , large_task>("Serial"              );
    benchmark_thread_pool<std_async_executor           , large_task>("std::async()"        );
    benchmark_thread_pool<parallel::thread_pool        , large_task>("parallel::ThreadPool");
    benchmark_thread_pool<bs_thread_pool_executor      , large_task>("BS::thread_pool"     );
    benchmark_thread_pool<progschj_thread_pool_executor, large_task>("progschj/ThreadPool" );
    
    bench.title("Shallow recursive tasks");
    benchmark_thread_pool<serial_executor      , shallow_recursive_task>("Serial"              );
    benchmark_thread_pool<std_async_executor   , shallow_recursive_task>("std::async()"        );
    benchmark_thread_pool<parallel::thread_pool, shallow_recursive_task>("parallel::ThreadPool");
    // others deadlock
    
    bench.title("Deep recursive tasks");
    benchmark_thread_pool<serial_executor      , deep_recursive_task>("Serial"              );
    benchmark_thread_pool<std_async_executor   , deep_recursive_task>("std::async()"        );
    benchmark_thread_pool<parallel::thread_pool, deep_recursive_task>("parallel::ThreadPool");
    // others deadlock
    // clang-format on
}