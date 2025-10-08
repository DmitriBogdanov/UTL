#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

#include <future> // async(), future<>

// ____________________ IMPLEMENTATION ____________________

// Tests styling stringification output

using namespace log::policy;

constexpr auto type   = Type::STREAM;
constexpr auto level  = Level::TRACE;
constexpr auto color  = Color::NONE;
constexpr auto format = Format::TITLE | Format::LEVEL;
// don't format time dependent fields that are not reproducible

template <Buffering buffering, Flushing flushing, Threading threading>
void test_config() {
    std::ostringstream oss;

    {
        auto logger = log::Logger{log::Sink<type, level, color, format, buffering, flushing, threading>{oss}};

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
                test_config<Buffering::NONE, Flushing::SYNC, Threading::UNSAFE>();
                test_config<Buffering::NONE, Flushing::ASYNC, Threading::UNSAFE>();
                test_config<Buffering::NONE, Flushing::SYNC, Threading::SAFE>();
                test_config<Buffering::NONE, Flushing::ASYNC, Threading::SAFE>();
                test_config<Buffering::FIXED, Flushing::SYNC, Threading::UNSAFE>();
                test_config<Buffering::FIXED, Flushing::ASYNC, Threading::UNSAFE>();
                test_config<Buffering::FIXED, Flushing::SYNC, Threading::SAFE>();
                test_config<Buffering::FIXED, Flushing::ASYNC, Threading::SAFE>();
                test_config<Buffering::TIMED, Flushing::SYNC, Threading::UNSAFE>();
                test_config<Buffering::TIMED, Flushing::ASYNC, Threading::UNSAFE>();
                test_config<Buffering::TIMED, Flushing::SYNC, Threading::SAFE>();
                test_config<Buffering::TIMED, Flushing::ASYNC, Threading::SAFE>();
            });
        }
        
        for (std::size_t task = 0; task < tasks_per_repeat; ++task) futures[task].wait();
    }
}