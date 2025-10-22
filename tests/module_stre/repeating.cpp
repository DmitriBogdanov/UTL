#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Other / Repeating") {
    CHECK(stre::repeat('k', 6) == "kkkkkk");
    CHECK(stre::repeat('\n', 4) == "\n\n\n\n");
    CHECK(stre::repeat('k', 0) == "");

    CHECK(stre::repeat("xo", 3) == "xoxoxo");
    CHECK(stre::repeat("a-\n-\t-b", 5) == "a-\n-\t-ba-\n-\t-ba-\n-\t-ba-\n-\t-ba-\n-\t-b");
    CHECK(stre::repeat("xo", 0) == "");
    CHECK(stre::repeat("", 0) == "");
    CHECK(stre::repeat("", 25) == "");
}