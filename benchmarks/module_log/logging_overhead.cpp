#include "benchmarks/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"

// Libraries to benchmarks against
// None

// Standard headers
#include <filesystem> // create_directories()

// ____________________ IMPLEMENTATION ____________________

// =================
// --- Benchmark ---
// =================

constexpr int repeats = 5'000;

void benchmark_logging_overhead_raw() {

    // Benchmark logging overhead with no decorators, just the message

    // Prepare data generation
    random::generators::RomuDuoJr64 gen;

    random::UniformIntDistribution  int_dist{-12345, 12345};
    random::UniformRealDistribution float_dist{-1e6, 1e6};

    // Random strings are pulled from cache to reduce overhead
    std::vector<std::string> cache(500);

    random::UniformIntDistribution<char>        char_dist{'a', 'z'};
    random::UniformIntDistribution<std::size_t> size_dist{2, 25};

    for (auto& str : cache) {
        str.resize(size_dist(gen));
        for (auto& ch : str) ch = char_dist(gen);
    }

    const auto random_string = [&] {
        random::UniformIntDistribution<std::size_t> pos_dist{0, cache.size() - 1};
        return cache[pos_dist(gen)];
    };

    // Create logging sinks
    auto logger = log::Logger{log::Sink<log::policy::Type::FILE, log::policy::Level::TRACE, log::policy::Color::NONE,
                                        log::policy::Format::NONE>{"temp/log1.log"}};

    std::ofstream log_file_2("temp/log2.log");
    std::ofstream log_file_3("temp/log3.log");

    // Benchmark
    bench.title("Raw logging overhead");

    benchmark("utl::log", [&]() {
        REPEAT(repeats)
        logger.trace("int = ", int_dist(gen), ", float = ", float_dist(gen), ", string = ", random_string());
    });

    benchmark("flushed std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file_2 << "int = " << int_dist(gen) << ", float = " << float_dist(gen) << ", string = " << random_string()
                   << '\n'
                   << std::flush;
    });

    benchmark("buffered std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file_3 << "int = " << int_dist(gen) << ", float = " << float_dist(gen) << ", string = " << random_string()
                   << '\n';
    });
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    std::filesystem::create_directories("temp"); // ensure proper directory exists for temp. files

    bench.timeUnit(1ns, "ns").epochIterations(10).warmup(10).relative(true); // global options

    benchmark_logging_overhead_raw();
}