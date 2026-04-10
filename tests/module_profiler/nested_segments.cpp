#include "tests/common.hpp"

#include "include/UTL/profiler.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// This test checks for name conflicts in nested profilers

TEST_CASE("Nested profiled segments / Same scope") {
    using namespace std::chrono_literals;

    UTL_PROFILER_BEGIN(segment_0, "label_0");

    std::this_thread::sleep_for(20ms);

    UTL_PROFILER_BEGIN(segment_1, "label_1");

    std::this_thread::sleep_for(20ms);

    UTL_PROFILER_BEGIN(segment_2, "label_2");

    std::this_thread::sleep_for(20ms);

    UTL_PROFILER_END(segment_2);

    UTL_PROFILER_END(segment_1);

    UTL_PROFILER_END(segment_0);
}

TEST_CASE("Nested profiled segments / Nested scopes") {
    using namespace std::chrono_literals;

    UTL_PROFILER_BEGIN(segment_0, "label_0");
    {
        std::this_thread::sleep_for(20ms);

        UTL_PROFILER_BEGIN(segment_1, "label_1");
        {
            std::this_thread::sleep_for(20ms);
            {
                UTL_PROFILER_BEGIN(segment_2, "label_2");

                std::this_thread::sleep_for(20ms);

                UTL_PROFILER_END(segment_2);
            }
        }
        UTL_PROFILER_END(segment_1);
    }
    UTL_PROFILER_END(segment_0);
}

TEST_CASE("Nested profiled segments / Nested scope captures") {
    using namespace std::chrono_literals;

    UTL_PROFILER("label_0") {
        std::this_thread::sleep_for(20ms);

        UTL_PROFILER("label_1") {
            std::this_thread::sleep_for(20ms);

            UTL_PROFILER("label_2") { std::this_thread::sleep_for(20ms); }
        }
    }
}