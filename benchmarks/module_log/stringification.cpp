#include "benchmarks/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"

// Libraries to benchmarks against
// None

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// =================
// --- Benchmark ---
// =================

constexpr int repeats = 30'000;

void benchmark_stringification_int() {

    random::generators::RomuDuoJr64 gen; // fast PRNG to minimize data generation overhead
    random::UniformIntDistribution  dist{-1234, 1234};

    bench.title("Stringifying bulk 'int' data");

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::stringify_append(str, dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << dist(gen);
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= (std::ostringstream{} << ...).str()", [&]() {
        std::string str;
        REPEAT(repeats) str += (std::ostringstream{} << dist(gen)).str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // Note:
    // += std::to_string() is surprisingly fast (I assume due to SSO) and sometimes even beats <charconv>.
    // I assume since there is no "variety" in how integers can be formatted, nothing prevent implementations
    // from using <charconv> algorithms inside the 'std::to_string()', in which case there is little room for
    // left further optimization
}


void benchmark_stringification_float() {

    random::generators::RomuDuoJr64 gen; // fast PRNG to minimize data generation overhead
    random::UniformRealDistribution dist{-1e6, 1e6};

    bench.title("Stringifying bulk 'double' data");

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::stringify_append(str, dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << dist(gen);
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= (std::ostringstream{} << ...).str()", [&]() {
        std::string str;
        REPEAT(repeats) str += (std::ostringstream{} << dist(gen)).str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
}

void benchmark_stringification_bool() {

    random::generators::RomuDuoJr64 gen; // fast PRNG to minimize data generation overhead
    random::UniformIntDistribution  dist{0U, 1U};

    bench.title("Stringifying bulk 'bool' data");

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::stringify_append(str, dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(dist(gen));
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (with std::boolalpha)", [&]() {
        std::ostringstream oss;
        oss << std::boolalpha;
        REPEAT(repeats) oss << dist(gen);
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
}

void benchmark_stringification_vector_of_strings() {

    random::generators::RomuDuoJr64 gen; // fast PRNG to minimize data generation overhead

    // Generate a "cache" of random vectors of strings so we can minimize the overhead of getting random data
    std::vector<std::vector<std::string>> cache(500);

    random::UniformIntDistribution<char>        char_dist{'a', 'z'};
    random::UniformIntDistribution<std::size_t> vec_size_dist{4, 16};
    random::UniformIntDistribution<std::size_t> str_size_dist{2, 25};

    for (auto& vec : cache) {
        vec.resize(vec_size_dist(gen));
        for (auto& str : vec) {
            str.resize(str_size_dist(gen));
            for (auto& ch : str) ch = char_dist(gen);
        }
    }

    const auto random_vector_of_strings = [&] {
        random::UniformIntDistribution<std::size_t> pos_dist{0, cache.size() - 1};
        return cache[pos_dist(gen)];
    };

    // Benchmark
    bench.title("Stringifying bulk 'std::vector<std::string>' data");

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::stringify_append(str, random_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(random_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (iterating the vector)", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) {
            const auto vec = random_vector_of_strings();
            oss << "{ ";
            for (auto it = vec.begin();;) {
                oss << *it;
                if (++it != vec.end()) oss << ", ";
                else break;
            }
            oss << " }";
        }
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
}

void benchmark_stringification_error_message() {
    // In this benchmark we want to format as simple error message and return it as
    // an 'std::string', stringstream will have to be temporary for that purpose

    random::generators::RomuDuoJr64 gen; // fast PRNG to minimize data generation overhead

    random::UniformIntDistribution char_dist{'1', '9'};
    random::UniformIntDistribution int_dist{-12345, 12345};

    bench.title("Format an error message").timeUnit(1ns, "ns").minEpochIterations(1000).warmup(10).relative(true);

    benchmark("log::stringify()", [&]() {
        std::string str = log::stringify("JSON object node encountered unexpected symbol {", char_dist(gen),
                                         "} after the pair key at pos ", int_dist(gen), " (should be {:}).");
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::string + std::string", [&]() {
        using namespace std::string_literals;
        std::string str = "JSON object node encountered unexpected symbol {" + std::to_string(char_dist(gen)) +
                          "} after the pair key at pos " + std::to_string(int_dist(gen)) + " (should be {:}).";
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        oss << "JSON object node encountered unexpected symbol {" << char_dist(gen) << "} after the pair key at pos "
            << int_dist(gen) << " (should be {:}).";
        std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    benchmark_stringification_int();
    benchmark_stringification_float();
    benchmark_stringification_bool();
    benchmark_stringification_vector_of_strings();
    benchmark_stringification_error_message();
}