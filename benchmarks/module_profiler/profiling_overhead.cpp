#include "benchmarks/common.hpp"

// #define UTL_PROFILER_USE_INTRINSICS_FOR_FREQUENCY 3.3e9 // uncomment to enable profiler intrinsics
#include "include/UTL/profiler.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"

// Libraries to benchmarks against
// None

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// #define BENCHMARK_RDTSC // uncomment to enable theoretical best intrinsic profiler

#ifdef BENCHMARK_RDTSC
#include <x86intrin.h>
#endif

// ==========================
// --- Profiled workloads ---
// ==========================

// Some kind of computation, heavy enough to be measurable, fast enough to not overshadow profiling overhead

const auto cos_x8 = [] {
    const double x1 = std::cos(random::uniform_double());
    const double x2 = std::cos(x1);
    const double x3 = std::cos(x2);
    const double x4 = std::cos(x3);
    const double x5 = std::cos(x4);
    const double x6 = std::cos(x5);
    const double x7 = std::cos(x6);
    const double x8 = std::cos(x7);
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
};

const auto cos_x32 = [] { return cos_x8() + cos_x8() + cos_x8() + cos_x8(); };

const auto fill_vector = [] {
    std::vector<double> vec(20);
    for (auto& e : vec) e = random::uniform_double();
    double sum = 0;
    for (auto& e : vec) sum += e * e;
    return sum;
};

// =================
// --- Benchmark ---
// =================

template <class Func>
void benchmark_profiling_overhead_on_workload(Func&& workload, const char* name) {
    constexpr int repeats = 50'000;

    bench.minEpochIterations(10).timeUnit(1ms, "ms").relative(true); // global options

    bench.title(name);

    benchmark("Runtime without profiling", [&]() {
        double sum = 0;
        REPEAT(repeats) { sum += workload(); }
        DO_NOT_OPTIMIZE_AWAY(sum);
    });

    benchmark("Theoretical best <chrono> profiler", [&]() {
        using clock = std::chrono::steady_clock;

        double          sum = 0;
        clock::duration elapsed{};

        REPEAT(repeats) {
            const auto start = clock::now();
            sum += workload();
            elapsed += clock::now() - start;
        }

        DO_NOT_OPTIMIZE_AWAY(sum);
        DO_NOT_OPTIMIZE_AWAY(elapsed);
    });

#ifdef BENCHMARK_RDTSC
    benchmark("Theoretical best __rdtsc() profiler", [&]() {
        double sum     = 0;
        auto   elapsed = 0ULL;

        REPEAT(repeats) {
            const auto start = __rdtsc();
            sum += workload();
            elapsed += __rdtsc() - start;
        }

        DO_NOT_OPTIMIZE_AWAY(sum);
        DO_NOT_OPTIMIZE_AWAY(elapsed);
    });
#endif

    benchmark("UTL_PROFILER()", [&]() {
        double sum = 0;
        REPEAT(repeats) { UTL_PROFILER("Work profiler") sum += workload(); }
        DO_NOT_OPTIMIZE_AWAY(sum);
    });
}

#define BENCHMARK_PROFILING_OVERHEAD_ON_WORKLOAD(func_) benchmark_profiling_overhead_on_workload(func_, #func_)

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    BENCHMARK_PROFILING_OVERHEAD_ON_WORKLOAD(cos_x8);
    BENCHMARK_PROFILING_OVERHEAD_ON_WORKLOAD(cos_x32);
    BENCHMARK_PROFILING_OVERHEAD_ON_WORKLOAD(fill_vector);
}