#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Padding") {
    CHECK(stre::pad_left("XXX", 6) == "   XXX");
    CHECK(stre::pad_left("XXX", 3) == "XXX");
    CHECK(stre::pad_left("XXX", 0) == "XXX");
    CHECK(stre::pad_left("XXX", 6, '-') == "---XXX");

    CHECK(stre::pad_right("XXX", 6) == "XXX   ");
    CHECK(stre::pad_right("XXX", 3) == "XXX");
    CHECK(stre::pad_right("XXX", 0) == "XXX");
    CHECK(stre::pad_right("XXX", 6, '-') == "XXX---");

    CHECK(stre::pad("XXX", 9) == "   XXX   ");
    CHECK(stre::pad("XXX", 8) == "  XXX   ");
    CHECK(stre::pad("XXX", 3) == "XXX");
    CHECK(stre::pad("XXX", 0) == "XXX");
    CHECK(stre::pad("XXX", 9, '-') == "---XXX---");
}