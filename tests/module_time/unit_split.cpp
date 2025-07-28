#include "tests/common.hpp"

#include "include/UTL/time.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

using namespace std::chrono_literals;

TEST_CASE("Unit split / Splitting") {
    constexpr auto duration = 11ns + 22us + 33ms + 44s + 55min + 66h;

    constexpr auto split = time::unit_split(duration);

    static_assert(split.ns == 11ns);
    static_assert(split.us == 22us);
    static_assert(split.ms == 33ms);
    static_assert(split.sec == 44s);
    static_assert(split.min == 55min);
    static_assert(split.hours == 66h);
}

TEST_CASE("Unit split / Stringification") {
    constexpr auto duration = 11ns + 22us + 33ms + 44s + 55min + 66h;
    
    CHECK(time::to_string(duration, 0) == "");
    CHECK(time::to_string(duration, 1) == "66 hours");
    CHECK(time::to_string(duration, 2) == "66 hours 55 min");
    CHECK(time::to_string(duration, 3) == "66 hours 55 min 44 sec");
    CHECK(time::to_string(duration, 4) == "66 hours 55 min 44 sec 33 ms");
    CHECK(time::to_string(duration, 5) == "66 hours 55 min 44 sec 33 ms 22 us");
    CHECK(time::to_string(duration, 6) == "66 hours 55 min 44 sec 33 ms 22 us 11 ns");
}