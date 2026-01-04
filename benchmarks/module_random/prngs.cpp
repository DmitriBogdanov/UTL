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

// =============================================
// --- C-style RNG wrapped into C++ PRNG API ---
// =============================================

struct rand_wrapper {
    using result_type = int;

    rand_wrapper(unsigned int seed) { std::srand(seed); };

    static constexpr result_type min() noexcept { return 0; }
    static constexpr result_type max() noexcept { return RAND_MAX; }

    int operator()() { return std::rand(); }
};

// =================
// --- Benchmark ---
// =================

constexpr std::uint32_t rand_seed = 15;
constexpr std::size_t   data_size = 5'000'000;

template <class Generator>
void benchmark_prng(const char* name) {
    std::vector<typename Generator::result_type> data(data_size);

    Generator gen{rand_seed};

    benchmark(name, [&] {
        for (auto& e : data) e = gen();
        DO_NOT_OPTIMIZE_AWAY(data);
    });
}

#define BENCHMARK_PRNG(gen_) benchmark_prng<gen_>(#gen_)

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    println("\n\n====== BENCHMARKING: PRNG invocation ======\n");
    println("N                -> ", data_size);
    println("Max memory usage -> ", data_size * sizeof(std::uint64_t) / 1e6, " MB");
    println("Min memory usage -> ", data_size * sizeof(std::uint16_t) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    // clang-format off
    bench.title("PRNG invocation");
    
    BENCHMARK_PRNG(std::minstd_rand0                 ); // same results as minstd_rand
    BENCHMARK_PRNG(std::minstd_rand                  );
    BENCHMARK_PRNG(rand_wrapper                      );
    BENCHMARK_PRNG(std::mt19937                      );
    BENCHMARK_PRNG(std::mt19937_64                   );
    // BENCHMARK_PRNG(std::ranlux24_base                ); // VERY slow, ~5%, not worth running
    // BENCHMARK_PRNG(std::ranlux48_base                ); // VERY slow, ~5%, not worth running
    BENCHMARK_PRNG(std::ranlux24                     );
    BENCHMARK_PRNG(std::ranlux48                     );
    BENCHMARK_PRNG(std::knuth_b                      );
    
    BENCHMARK_PRNG(random::generators::romu_mono_16  );
    BENCHMARK_PRNG(random::generators::romu_trio_32  );
    BENCHMARK_PRNG(random::generators::splitmix_32   );
    BENCHMARK_PRNG(random::generators::xoshiro_128   );
    BENCHMARK_PRNG(random::generators::romu_duo_jr_64);
    BENCHMARK_PRNG(random::generators::splitmix_64   );
    BENCHMARK_PRNG(random::generators::xoshiro_256   );
    BENCHMARK_PRNG(random::generators::chacha_8      );
    BENCHMARK_PRNG(random::generators::chacha_12     );
    BENCHMARK_PRNG(random::generators::chacha_20     );
    // clang-format on
}