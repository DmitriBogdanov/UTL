#include "benchmarks/common.hpp"

#include "include/UTL/random.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
// None

// Libraries to benchmarks against
// None

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// =================
// --- Benchmark ---
// =================

constexpr std::uint32_t rand_seed = 15;
constexpr std::size_t   data_size = 5'000'000;
constexpr double        min       = 4.;
constexpr double        max       = 117.;

template <class Dist, class Gen>
void benchmark_distribution_for_prng(const char* name) {
    std::vector<typename Dist::result_type> data(data_size);

    Gen  gen{rand_seed};
    Dist dist{min, max};

    benchmark(name, [&] {
        for (auto& e : data) e = dist(gen);
        DO_NOT_OPTIMIZE_AWAY(data);
    });
}

#define BENCHMARK_DISTRIBUTION_FOR_PRNG(dist_, gen_) benchmark_distribution_for_prng<dist_, gen_>(#dist_)

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    println("\n\n====== BENCHMARKING: Normal distribution ======\n");
    println("N                -> ", data_size);
    println("Max memory usage -> ", data_size * sizeof(double) / 1e6, " MB");
    println("Min memory usage -> ", data_size * sizeof(float) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    // clang-format off
    bench.title("64-bit float distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(        std::normal_distribution<double>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(      random::NormalDistribution<double>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::ApproxNormalDistribution<double>, random::generators::SplitMix64);
    
    bench.title("64-bit float distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(        std::normal_distribution<double>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(      random::NormalDistribution<double>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::ApproxNormalDistribution<double>, random::generators::SplitMix64);
    
    bench.title("32-bit float distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(        std::normal_distribution< float>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(      random::NormalDistribution< float>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::ApproxNormalDistribution< float>, random::generators::SplitMix64);
    
    bench.title("32-bit float distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(        std::normal_distribution< float>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(      random::NormalDistribution< float>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::ApproxNormalDistribution< float>, random::generators::SplitMix32);
    // clang-format on
}