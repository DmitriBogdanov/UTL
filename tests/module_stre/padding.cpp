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
    
    CHECK(stre::pad_with_leading_zeroes(15, 4) == "0015");
    CHECK(stre::pad_with_leading_zeroes(174, 10) == "0000000174");
    CHECK(stre::pad_with_leading_zeroes(137, 3) == "137");
    CHECK(stre::pad_with_leading_zeroes(4321, 2) == "4321");
}