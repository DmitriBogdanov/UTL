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
    println("\n\n====== BENCHMARKING: Uniform float distribution ======\n");
    println("N                -> ", data_size);
    println("Max memory usage -> ", data_size * sizeof(double) / 1e6, " MB");
    println("Min memory usage -> ", data_size * sizeof(float) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    // clang-format off
    bench.title("64-bit float distribution | splitmix_64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(   std::uniform_real_distribution<double>, random::generators::splitmix_64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::uniform_real_distribution<double>, random::generators::splitmix_64);
    
    bench.title("64-bit float distribution | splitmix_32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(   std::uniform_real_distribution<double>, random::generators::splitmix_32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::uniform_real_distribution<double>, random::generators::splitmix_32);
    
    bench.title("32-bit float distribution | splitmix_64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(   std::uniform_real_distribution< float>, random::generators::splitmix_64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::uniform_real_distribution< float>, random::generators::splitmix_64);
    
    bench.title("32-bit float distribution | splitmix_32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG(   std::uniform_real_distribution< float>, random::generators::splitmix_32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::uniform_real_distribution< float>, random::generators::splitmix_32);
    // clang-format on
}