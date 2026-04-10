#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

#include <future> // async(), future<>

// ____________________ IMPLEMENTATION ____________________

// Tests styling stringification output

namespace pl = log::policy;

constexpr auto type   = pl::type::stream;
constexpr auto level  = pl::level::trace;
constexpr auto color  = pl::color::none;
constexpr auto format = pl::format::title | pl::format::level;
// don't format time dependent fields that are not reproducible

template <pl::buffering buffering, pl::flushing flushing, pl::threading threading>
void test_config() {
    std::ostringstream oss;

    {
        auto logger = log::logger{log::sink<type, level, color, format, buffering, flushing, threading>{oss}};

        logger.info("Message 1");
        logger.note("Message 2");
        logger.warn("Message 3");
    }

    const std::string expected = R"(| level | message)"
                                 "\n"
                                 R"(| ----- | ------------------------------)"
                                 "\n"
                                 R"(|  INFO | Message 1)"
                                 "\n"
                                 R"(|  NOTE | Message 2)"
                                 "\n"
                                 R"(|  WARN | Message 3)"
                                 "\n";

    const std::string result = oss.str();

    CHECK(result.size() == expected.size());

    CHECK("\n[START]\n" + result + "\n[END]\n" == "\n[START]\n" + expected + "\n[END]\n");
    // prefix & suffix with newlines make the output more readable in the case of a failure
}

TEST_CASE("Sinks / Configuration") {
    // 3 x 2 x 2 = 12 different configuration to test, all of them should output the same string,
    // repeat multiple times on multiple threads to increase the chance of catching threading issues
    constexpr std::size_t repeats          = 4;
    constexpr std::size_t tasks_per_repeat = 20;

    for (std::size_t repeat = 0; repeat < repeats; ++repeat) {
        std::array<std::future<void>, tasks_per_repeat> futures;

        for (std::size_t task = 0; task < tasks_per_repeat; ++task) {
            println("========================\n> Repeat = ", repeat, ", task = ", task);

            futures[task] = std::async([] {
                // clang-format off
                test_config<pl::buffering::none , pl::flushing::sync , pl::threading::unsafe>();
                test_config<pl::buffering::none , pl::flushing::async, pl::threading::unsafe>();
                test_config<pl::buffering::none , pl::flushing::sync , pl::threading::safe  >();
                test_config<pl::buffering::none , pl::flushing::async, pl::threading::safe  >();
                test_config<pl::buffering::fixed, pl::flushing::sync , pl::threading::unsafe>();
                test_config<pl::buffering::fixed, pl::flushing::async, pl::threading::unsafe>();
                test_config<pl::buffering::fixed, pl::flushing::sync , pl::threading::safe  >();
                test_config<pl::buffering::fixed, pl::flushing::async, pl::threading::safe  >();
                test_config<pl::buffering::timed, pl::flushing::sync , pl::threading::unsafe>();
                test_config<pl::buffering::timed, pl::flushing::async, pl::threading::unsafe>();
                test_config<pl::buffering::timed, pl::flushing::sync , pl::threading::safe  >();
                test_config<pl::buffering::timed, pl::flushing::async, pl::threading::safe  >();
                // clang-format on
            });
        }

        for (std::size_t task = 0; task < tasks_per_repeat; ++task) futures[task].wait();
    }
}