// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string_view>
#include <thread>
#include <variant>

// ________________ BENCHMARK INCLUDES ________________

#include <cmath>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

// #define BENCHMARK_RDTSC

#ifdef BENCHMARK_RDTSC
#include <x86intrin.h> // testing
#endif

const auto compute_value = []() {
    const auto x1 = std::cos(utl::random::uniform_double());
    const auto x2 = std::cos(x1);
    const auto x3 = std::cos(x2);
    const auto x4 = std::cos(x3);
    const auto x5 = std::cos(x4);
    const auto x6 = std::cos(x5);
    const auto x7 = std::cos(x6);
    const auto x8 = std::cos(x7);
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
};
// some kind of computation, just heavy enough to be measurable, yet fast enough to not overshadow profiling overhead

void benchmark_profiling_overhead() {
    constexpr int repeats = 50'000;

    bench.minEpochIterations(10).timeUnit(1ms, "ms").relative(true);

    benchmark("Runtime without profiling", [&]() {
        double s = 0.;
        REPEAT(repeats) { s += compute_value(); }
        DO_NOT_OPTIMIZE_AWAY(s);
    });

    benchmark("Theoretical best std::chrono profiler", [&]() {
        double                   s = 0.;
        std::chrono::nanoseconds time{};
        REPEAT(repeats) {
            const auto start = std::chrono::steady_clock::now();
            s += compute_value();
            time += std::chrono::steady_clock::now() - start;
        }
        DO_NOT_OPTIMIZE_AWAY(s);
        DO_NOT_OPTIMIZE_AWAY(time);
    });

#ifdef BENCHMARK_RDTSC
    benchmark("Theoretical best __rdtsc() profiler", [&]() {
        double   s = 0.;
        uint64_t time{};
        REPEAT(repeats) {
            const uint64_t start = __rdtsc();
            s += compute_value();
            time += __rdtsc() - start;
        }
        DO_NOT_OPTIMIZE_AWAY(s);
        DO_NOT_OPTIMIZE_AWAY(time);
    });
#endif

    benchmark("UTL_PROFILER()", [&]() {
        double s = 0.;
        REPEAT(repeats) { UTL_PROFILER("Work profiler") s += compute_value(); }
        DO_NOT_OPTIMIZE_AWAY(s);
    });

    // Here 'theoretical best profiler' is the one that has no additional overhead besides
    // the time measurement at two points, can't get better than without swithing to a
    // completely different profiling method (like, for example, sampling or CPU instruction modeling)
}

void test_scope_profiler_precision() {
    UTL_PROFILER("Scope precision test:   50 ms") utl::sleep::spinlock(50ms);
    UTL_PROFILER("Scope precision test:  200 ms") utl::sleep::spinlock(200ms);
    UTL_PROFILER("Scope precision test: 1000 ms") utl::sleep::spinlock(1000ms);
}

void test_segment_profiler_precision() {
    UTL_PROFILER_BEGIN(segment_1, "Segment precision test:   50 ms");
    utl::sleep::spinlock(50ms);
    UTL_PROFILER_END(segment_1);

    UTL_PROFILER_BEGIN(segment_2, "Segment precision test:   200 ms");
    utl::sleep::spinlock(200ms);
    UTL_PROFILER_END(segment_2);

    UTL_PROFILER_BEGIN(segment_3, "Segment precision test:  1000 ms");
    utl::sleep::spinlock(1000ms);
    UTL_PROFILER_END(segment_3);
}

int main() {

    UTL_PROFILER("Top level profiler")
    benchmark_profiling_overhead();

    // test_scope_profiler_precision();
    // test_segment_profiler_precision();
}