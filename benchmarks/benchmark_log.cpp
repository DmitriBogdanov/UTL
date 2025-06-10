// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include <chrono>
#include <climits>
#include <complex>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

// ==================================
// --- Stringification benchmarks ---
// ==================================

void benchmark_stringification() {
    using namespace utl;

    constexpr int repeats = 20'000;

    bench.timeUnit(1ms, "ms").minEpochIterations(20);

    // --- Integer stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'int' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, datagen::rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(datagen::rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(datagen::rand_int());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << datagen::rand_int();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
    
    benchmark("+= (std::ostringstream{} << ...).str()", [&]() {
        std::string str;
        REPEAT(repeats) str += (std::ostringstream{} << datagen::rand_int()).str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // Note:
    // += std::to_string() is surprisingly fast (I assume due to SSO) and sometimes even beats <charconv>.
    // I assume since there is no "variety" in how integers can be formatted, nothing prevent implementations
    // from using <charconv> algorithms inside the 'std::to_string()', in which case there is little room for
    // further optimization left.

    // --- Float stringification ---
    // ------------------------------
    bench.title("Stringify bulk 'double' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, datagen::rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(datagen::rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string()", [&]() {
        std::string str;
        REPEAT(repeats) str += std::to_string(datagen::rand_double());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << datagen::rand_double();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });
    
    benchmark("+= (std::ostringstream{} << ...).str()", [&]() {
        std::string str;
        REPEAT(repeats) str += (std::ostringstream{} << datagen::rand_double()).str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Complex stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'std::complex<double>' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, datagen::rand_complex());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(datagen::rand_complex());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= std::to_string() (component-wise)", [&]() {
        std::string str;
        REPEAT(repeats) {
            str += std::to_string(datagen::rand_complex().real());
            str += " + ";
            str += std::to_string(datagen::rand_complex().imag());
            str += " i";
        }
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) oss << datagen::rand_complex();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Bool stringification ---
    // -------------------------------
    bench.title("Stringify bulk 'bool' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, datagen::rand_bool());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(datagen::rand_bool());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (with std::boolalpha)", [&]() {
        std::ostringstream oss;
        oss << std::boolalpha;
        REPEAT(repeats) oss << datagen::rand_bool();
        const std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Vector of strings stringification ---
    // -----------------------------------------
    bench.title("Stringify bulk 'std::vector<std::string>' data").warmup(10).relative(true);

    benchmark("log::append_stringified()", [&]() {
        std::string str;
        REPEAT(repeats) log::append_stringified(str, datagen::rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("+= log::stringify()", [&]() {
        std::string str;
        REPEAT(repeats) str += log::stringify(datagen::rand_vector_of_strings());
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream << (iterating the vector)", [&]() {
        std::ostringstream oss;
        REPEAT(repeats) {
            const auto vec = datagen::rand_vector_of_strings();
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

    // --- Error message stringification ---
    // -------------------------------------

    // In this benchmark we want to format as simple error message and return it as 'std::string',
    // stringstream will have to be temporary for that purpose
    bench.title("Format an error message").timeUnit(1ns, "ns").minEpochIterations(1000).warmup(10).relative(true);

    benchmark("log::stringify()", [&]() {
        std::string str = log::stringify("JSON object node encountered unexpected symbol {", datagen::rand_char(),
                                         "} after the pair key at pos ", datagen::rand_int(), " (should be {:}).");
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::string + std::string", [&]() {
        using namespace std::string_literals;
        std::string str = "JSON object node encountered unexpected symbol {" + std::to_string(datagen::rand_int()) +
                          "} after the pair key at pos " + std::to_string(datagen::rand_int()) + " (should be {:}).";
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    benchmark("std::ostringstream <<", [&]() {
        std::ostringstream oss;
        oss << "JSON object node encountered unexpected symbol {" << datagen::rand_char()
            << "} after the pair key at pos " << datagen::rand_int() << " (should be {:}).";
        std::string str = oss.str();
        DO_NOT_OPTIMIZE_AWAY(str);
    });

    // --- Left-pad formatting ---
    // ---------------------------
    bench.title("Left-pad a string").timeUnit(1ns, "ns").minEpochIterations(20).warmup(10).relative(true);

    constexpr std::size_t pad_size = 20;

    std::vector<std::string> strings(10'000);
    const auto               reset_strings = [&] {
        for (auto& e : strings) e = "";
    };

    reset_strings();
    benchmark("log::stringify(log::PadLeft{})", [&]() {
        for (auto& e : strings) e = log::stringify(log::PadLeft{datagen::rand_string(), pad_size});
        DO_NOT_OPTIMIZE_AWAY(strings);
    });

    reset_strings();
    benchmark("Manual alignment with std::string.append()", [&]() {
        for (auto& e : strings) {
            const std::string temp = datagen::rand_string();
            if (temp.size() < pad_size) e.append(pad_size - temp.size(), ' ');
            e += temp;
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });

    reset_strings();
    benchmark("std::ostringstream << std::setw() << std::right", [&]() {
        for (auto& e : strings) {
            std::ostringstream oss;
            oss << std::setw(pad_size) << std::right << datagen::rand_string();
            e = oss.str();
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });

    // --- Right-pad formatting ---
    // ---------------------------
    bench.title("Right-pad a string").timeUnit(1ns, "ns").minEpochIterations(20).warmup(10).relative(true);

    reset_strings();
    benchmark("log::stringify(log::PadRight{})", [&]() {
        for (auto& e : strings) e = log::stringify(log::PadRight{datagen::rand_string(), pad_size});
        DO_NOT_OPTIMIZE_AWAY(strings);
    });

    reset_strings();
    benchmark("Manual alignment with std::string.append()", [&]() {
        for (auto& e : strings) {
            e = datagen::rand_string();
            if (e.size() < pad_size) e.append(pad_size - e.size(), ' ');
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });

    reset_strings();
    benchmark("std::ostringstream << std::setw() << std::left", [&]() {
        for (auto& e : strings) {
            std::ostringstream oss;
            oss << std::setw(pad_size) << std::left << datagen::rand_string();
            e = oss.str();
        }
        DO_NOT_OPTIMIZE_AWAY(strings);
    });
}

// ==========================
// --- Logging benchmarks ---
// ==========================

void benchmark_raw_logging_overhead() {
    using namespace utl;

    // Benchmark logging
    bench.title("Logging").timeUnit(1ns, "ns").epochIterations(10).warmup(10).relative(true);

    constexpr int repeats = 5'000;

    log::Columns cols;
    cols.datetime = false;
    cols.uptime   = false;
    cols.thread   = false;
    cols.callsite = false;
    cols.level    = false;
    log::add_file_sink("temp/log1.log").set_columns(cols).set_flush_interval(std::chrono::nanoseconds{5000});

    std::ofstream log_file_2("temp/log2.log");
    std::ofstream log_file_3("temp/log3.log");

    benchmark("utl::log", [&]() {
        REPEAT(repeats)
        UTL_LOG_TRACE("int = ", datagen::rand_int(), ", float = ", datagen::rand_double(),
                      ", string = ", datagen::rand_string());
    });

    benchmark("flushed std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file_2 << "int = " << datagen::rand_int() << ", float = " << datagen::rand_double()
                   << ", string = " << datagen::rand_string() << '\n'
                   << std::flush;
    });

    benchmark("buffered std::ostream::<<", [&]() {
        REPEAT(repeats)
        log_file_3 << "int = " << datagen::rand_int() << ", float = " << datagen::rand_double()
                   << ", string = " << datagen::rand_string() << '\n';
    });
}

int main() {
    using namespace utl;

    benchmark_stringification();
    //benchmark_raw_logging_overhead();
}