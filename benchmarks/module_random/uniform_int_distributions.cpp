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

// ===============================================
// --- Additional distribution implementations ---
// ===============================================

// clang-format off
template <class> constexpr bool always_false_v = false;

template<class T> struct wider { static_assert(always_false_v<T>, "Missing specialization"); };

template<> struct wider<std::uint8_t > { using type = std::uint16_t;         };
template<> struct wider<std::uint16_t> { using type = std::uint32_t;         };
template<> struct wider<std::uint32_t> { using type = std::uint64_t;         };
template<> struct wider<std::uint64_t> { using type = random::impl::Uint128; };

template<class T> using wider_t = typename wider<T>::type;
// clang-format on

template <class T>
struct UintBiasedBigMulDist {
    using result_type = T;

    using narrow = T;
    using wide   = wider_t<T>;

    narrow min;
    narrow max;

    constexpr UintBiasedBigMulDist(T min, T max) noexcept : min(min), max(max) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        const narrow range = this->max - this->min;
        const wide   mul   = wide(gen()) * wide(range);
        const narrow rng   = mul >> std::numeric_limits<narrow>::digits;
        return this->min + rng;
    } // similar to Lemire's algorithm, but without de-biasing, very fast
};

// =================
// --- Benchmark ---
// =================

constexpr std::uint32_t rand_seed = 15;
constexpr std::size_t   data_size = 5'000'000;
constexpr std::uint32_t min       = 4;
constexpr std::uint32_t max       = 117;

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
    println("\n\n====== BENCHMARKING: Uniform int distribution ======\n");
    println("N                -> ", data_size);
    println("Max memory usage -> ", data_size * sizeof(std::uint64_t) / 1e6, " MB");
    println("Min memory usage -> ", data_size * sizeof(std::uint8_t) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    // clang-format off
    bench.title("64-bit uint distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint64_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint64_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint64_t>, random::generators::SplitMix64);
    
    bench.title("64-bit uint distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint64_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint64_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint64_t>, random::generators::SplitMix32);
    
    bench.title("32-bit uint distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint32_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint32_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint32_t>, random::generators::SplitMix64);
    
    bench.title("32-bit uint distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint32_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint32_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint32_t>, random::generators::SplitMix32);
    
    bench.title("16-bit uint distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint16_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint16_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint16_t>, random::generators::SplitMix64);
    
    bench.title("16-bit uint distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution<std::uint16_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution<std::uint16_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist<std::uint16_t>, random::generators::SplitMix32);
    
    bench.title("8-bit uint distribution | SplitMix64");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution< std::uint8_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution< std::uint8_t>, random::generators::SplitMix64);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist< std::uint8_t>, random::generators::SplitMix64);
    
    bench.title(" 8-bit uint distribution | SplitMix32");
    BENCHMARK_DISTRIBUTION_FOR_PRNG( std::uniform_int_distribution< std::uint8_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(random::UniformIntDistribution< std::uint8_t>, random::generators::SplitMix32);
    BENCHMARK_DISTRIBUTION_FOR_PRNG(          UintBiasedBigMulDist< std::uint8_t>, random::generators::SplitMix32);
    // clang-format on
}