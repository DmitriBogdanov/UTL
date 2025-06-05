// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

// =======================
// --- PRNG Benchmarks ---
// =======================

struct RandWrapper {
    using result_type = int;

    RandWrapper(unsigned int seed) { std::srand(seed); };

    static constexpr result_type min() noexcept { return 0; }
    static constexpr result_type max() noexcept { return RAND_MAX; }

    int operator()() { return std::rand(); }
}; // std-engine-like wrapper for 'rand()' used for the benchmarks

constexpr std::uint32_t rand_seed = 15;
constexpr std::size_t   data_size = 5'000'000;

template <class Generator>
void benchmark_prng(const char* name) {
    Generator gen{rand_seed};

    std::vector<typename Generator::result_type> data(data_size);

    benchmark(name, [&] {
        for (auto& e : data) e = gen();
    });
}

void benchmark_prngs() {
    using namespace utl;

    log::println("\n\n====== BENCHMARKING: PRNG invocation ======\n");
    log::println("N                 -> ", data_size);
    log::println("Max memory usage -> ", math::to_memory_units(data_size * sizeof(std::uint64_t)), " MiB");
    log::println("Min memory usage -> ", math::to_memory_units(data_size * sizeof(std::uint16_t)), " MiB");

    bench.title(std::to_string(data_size) + " invocations of PRNG")
        .timeUnit(1ms, "ms")
        .minEpochIterations(5)
        .warmup(10)
        .relative(true);

    // 'std::' PRNGs
    // benchmark_prng<std::minstd_rand0>("std::minstd_rand0"); // same results as minstd_rand
    benchmark_prng<std::minstd_rand>("std::minstd_rand");
    benchmark_prng<RandWrapper>("rand()");
    benchmark_prng<std::mt19937>("std::mt19937");
    benchmark_prng<std::mt19937_64>("std::mt19937_64");
    // benchmark_prng<std::ranlux24_base>("ranlux24_base");
    // benchmark_prng<std::ranlux48_base>("ranlux48_base");
    // benchmark_prng<std::ranlux24>("ranlux24"); // too slow to even measure, about ~5% performance of minstd_rand
    // benchmark_prng<std::ranlux48>("ranlux48"); // too slow to even measure, about ~3% performance of minstd_rand
    benchmark_prng<std::knuth_b>("knuth_b");

    // 'utl::' PRNGs
    benchmark_prng<random::generators::RomuMono16>("RomuMono");
    benchmark_prng<random::generators::RomuTrio32>("RomuTrio32");
    benchmark_prng<random::generators::SplitMix32>("SplitMix32");
    benchmark_prng<random::generators::Xoshiro128PP>("Xoshiro128++");
    benchmark_prng<random::generators::RomuDuoJr64>("RomuDuoJr64");
    benchmark_prng<random::generators::SplitMix64>("SplitMix64");
    benchmark_prng<random::generators::Xoshiro256PP>("Xoshiro256++");
    benchmark_prng<random::generators::ChaCha8>("ChaCha8");
    benchmark_prng<random::generators::ChaCha12>("ChaCha12");
    benchmark_prng<random::generators::ChaCha20>("ChaCha20");
}

// =======================================
// --- Uniform Distribution benchmarks ---
// =======================================

template <class T>
constexpr bool always_false_v = false;

// clang-format off
template<class T> struct wider { static_assert(always_false_v<T>, "Missing specialization"); };

template<> struct wider<std::uint8_t > { using type = std::uint16_t; };
template<> struct wider<std::uint16_t> { using type = std::uint32_t; };
template<> struct wider<std::uint32_t> { using type = std::uint64_t; };
template<> struct wider<std::int8_t  > { using type =  std::int16_t; };
template<> struct wider<std::int16_t > { using type =  std::int32_t; };
template<> struct wider<std::int32_t > { using type =  std::int64_t; };

template<class T> struct narrower_t { static_assert(always_false_v<T>, "Missing specialization"); };


template<> struct narrower_t< std::uint8_t> { using type =          void; };
template<> struct narrower_t<std::uint16_t> { using type =  std::uint8_t; };
template<> struct narrower_t<std::uint32_t> { using type = std::uint16_t; };
template<> struct narrower_t<std::uint64_t> { using type = std::uint32_t; };

#ifdef __SIZEOF_INT128__
template<> struct wider<std::uint64_t> { using type = __uint128_t  ; };
template<> struct wider<std::int64_t > { using type =  __int128_t  ; }; 
#else
template<> struct wider<std::uint64_t> { using type = void         ; };
template<> struct wider<std::int64_t > { using type = void         ; }; 
#endif // GCC extension, libstdc++ Lemire's algorithm uses these

template<class T> using wider_t = typename wider<T>::type;

template<class T> constexpr auto type_name = "unknown";

template<> constexpr auto type_name<std::uint8_t > = "uint8"  ;
template<> constexpr auto type_name<std::uint16_t> = "uint16" ;
template<> constexpr auto type_name<std::uint32_t> = "uint32" ;
template<> constexpr auto type_name<std::uint64_t> = "uint64" ;
template<> constexpr auto type_name<__uint128_t  > = "uint128";
template<> constexpr auto type_name<std::int8_t  > =  "int8"  ;
template<> constexpr auto type_name<std::int16_t > =  "int16" ;
template<> constexpr auto type_name<std::int32_t > =  "int32" ;
template<> constexpr auto type_name<std::int64_t > =  "int64" ;
template<> constexpr auto type_name<__int128_t   > =  "int128";

template<> constexpr auto type_name<float      > =  "float"      ;
template<> constexpr auto type_name<double     > =  "double"     ;
template<> constexpr auto type_name<long double> =  "long double";
// clang-format on

// --- Test Distributions ---
// --------------------------

// Below are a few not-entirely-correct distribution implementations
// that were used to estimate the performance of different algorithms
// before committing to a proper implementation in the module

template <class T>
struct uint_dist_lemire {
    using narrow = T;
    using wide   = wider_t<T>;

    narrow range;

    constexpr uint_dist_lemire(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        wide   product = wide(gen()) * wide(range);
        narrow low     = narrow(product);
        if (low < range) {
            const narrow threshold = -range % range;
            while (low < threshold) {
                product = wide(gen()) * wide(range); // Note: Caching this value as a member makes no difference
                low     = narrow(product);
            }
        }
        return product >> std::numeric_limits<narrow>::digits;
    }
};

template <class T>
struct uint_dist_mod_1x {
    T range;

    constexpr uint_dist_mod_1x(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        T x{}, r{};
        do {
            x = gen();
            r = x % range;
        } while (x - r > T(-range));
        return r;
    }
};

template <class T>
struct uint_dist_biased_mult {
    using narrow = T;
    using wide   = wider_t<T>;

    narrow range;

    constexpr uint_dist_biased_mult(T range) noexcept : range(range) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        const wide mul = wide(gen()) * wide(this->range);
        return mul >> std::numeric_limits<narrow>::digits;
    }
};

template <class T>
struct float_dist_std_canonical {
    T min, max;

    constexpr float_dist_std_canonical(T min, T max) noexcept : min(min), max(max) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        return min + std::generate_canonical<T, std::numeric_limits<T>::digits>(gen) * (max - min);
    }
};

template <class T>
struct float_dist_mult {
    T min, max;

    constexpr float_dist_mult(T min, T max) noexcept : min(min), max(max) {}

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept {
        const T canonical = (gen() >> 11) * 0x1.0p-53;
        return min + canonical * (max - min);
    }
}; // this is only 64-bit PRNG & 64-bit float implementation

// --- Benchmark distributions ---
// -------------------------------

constexpr std::uint64_t uint_min = 3;
constexpr std::uint64_t uint_max = 47;

constexpr double real_min = -3.15;
constexpr double real_max = 147.54;

template <class Generator>
void benchmark_distributions_for_prng(const char* name) {
    Generator gen{rand_seed};

    // uints
    [[maybe_unused]] const auto bench_uints = [&](auto min, auto max) {
        using uint = decltype(min);

        const uint range = max - min;

        std::vector<uint> data(data_size);

        bench.title(type_name<uint> + std::string(" distribution with ") + name)
            .timeUnit(1ms, "ms")
            .minEpochIterations(5)
            .warmup(10)
            .relative(true);

        benchmark("std::uniform_int_distribution", [&] {
            if constexpr (sizeof(uint) >= sizeof(unsigned short)) {
                std::uniform_int_distribution dist{min, max};
                for (auto&& e : data) e = dist(gen);
            } else {
                std::uniform_int_distribution<int> dist{min, max};
                for (auto&& e : data) e = dist(gen);
            }
        });

        benchmark("uint_dist_lemire", [&] {
            const uint_dist_lemire dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("uint_dist_mod_1x", [&] {
            const uint_dist_mod_1x dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });
        
        benchmark("uint_dist_biased_mult", [&] {
            const uint_dist_biased_mult dist{range};
            for (auto&& e : data) e = min + dist(gen);
        });

        benchmark("UniformIntDistribution", [&] {
            const random::UniformIntDistribution dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });
        
        benchmark("uniform<>(min, max)", [&] {
            for (auto&& e : data) e = random::uniform(min, max);
        });
    };

    [[maybe_unused]] const auto bench_reals = [&](auto min, auto max) {
        using real = decltype(min);

        std::vector<real> data(data_size);

        bench.title(type_name<real> + std::string(" distribution with ") + name)
            .timeUnit(1ms, "ms")
            .minEpochIterations(5)
            .warmup(10)
            .relative(true);

        benchmark("std::uniform_real_distribution", [&] {
            std::uniform_real_distribution<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });

        benchmark("float_dist_std_canonical", [&] {
            const float_dist_std_canonical<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });

        if constexpr (sizeof(real) == 8 && sizeof(typename Generator::result_type) == 8) {
            benchmark("float_dist_mult", [&] {
                const float_dist_mult<real> dist{min, max};
                for (auto&& e : data) e = dist(gen);
            });
        }

        benchmark("UniformRealDistribution", [&] {
            const random::UniformRealDistribution<real> dist{min, max};
            for (auto&& e : data) e = dist(gen);
        });
        
        benchmark("uniform<>(min, max)", [&] {
            for (auto&& e : data) e = random::uniform(min, max);
        });
        
        benchmark("NormalDistribution", [&] {
            random::NormalDistribution<real> dist{5, 4};
            for (auto&& e : data) e = dist(gen);
        });
        
        benchmark("normal<>(mean, stddev)", [&] {
            for (auto&& e : data) e = random::normal<real>(5, 4);
        });
        
        benchmark("normal<>()", [&] {
            for (auto&& e : data) e = random::normal<real>();
        });
    };

    bench_uints(std::uint64_t(uint_min), std::uint64_t(uint_max));
    bench_uints(std::uint32_t(uint_min), std::uint32_t(uint_max));
    //bench_uints(std::uint16_t(uint_min), std::uint16_t(uint_max));
    //bench_uints(std::uint8_t(uint_min), std::uint8_t(uint_max));

    //bench_reals((long double)(real_min), (long double)(real_max));
    bench_reals(double(real_min), double(real_max));
    bench_reals(float(real_min), float(real_max));
}

void benchmark_distributions() {
    log::println("\n\n====== BENCHMARKING: distributions ======\n");
    log::println("N                          -> ", data_size);
    log::println("uint range                 -> [", uint_min, ", ", uint_max, "]");
    log::println("Memory usage (uint64)      -> ", math::to_memory_units(data_size * sizeof(std::uint64_t)), " MiB");
    log::println("Memory usage (uint32)      -> ", math::to_memory_units(data_size * sizeof(std::uint32_t)), " MiB");
    log::println("Memory usage (uint16)      -> ", math::to_memory_units(data_size * sizeof(std::uint32_t)), " MiB");
    log::println("Memory usage (uint8 )      -> ", math::to_memory_units(data_size * sizeof(std::uint8_t)), " MiB");
    log::println("real range                 -> [", real_min, ", ", real_max, "]");
    log::println("Memory usage (long double) -> ", math::to_memory_units(data_size * sizeof(long double)), " MiB");
    log::println("Memory usage (double     ) -> ", math::to_memory_units(data_size * sizeof(double)), " MiB");
    log::println("Memory usage (float      ) -> ", math::to_memory_units(data_size * sizeof(float)), " MiB");

    //benchmark_distributions_for_prng<std::minstd_rand>("std::minstd_rand");
    //benchmark_distributions_for_prng<std::mt19937>("std::mt19937");
    //benchmark_distributions_for_prng<std::mt19937_64>("std::mt19937_64");
    //benchmark_distributions_for_prng<random::generators::RomuMono16>("RomuMono16");
    //benchmark_distributions_for_prng<random::generators::RomuTrio32>("RomuTrio32");
    benchmark_distributions_for_prng<random::generators::SplitMix32>("SplitMix32");
    //benchmark_distributions_for_prng<random::generators::Xoshiro128PP>("Xoshiro128++");
    //benchmark_distributions_for_prng<random::generators::RomuDuoJr64>("RomuDuoJr64");
    benchmark_distributions_for_prng<random::generators::SplitMix64>("SplitMix64");
    benchmark_distributions_for_prng<random::generators::Xoshiro256PP>("Xoshiro256++");
    //benchmark_distributions_for_prng<random::generators::ChaCha20>("ChaCha20");
}


int main() {

    //benchmark_prngs();
    benchmark_distributions();

    return 0;
}