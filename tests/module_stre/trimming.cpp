#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Trimming") {
    CHECK(stre::trim_left("   XXX   ") == "XXX   ");
    CHECK(stre::trim_left("XXX") == "XXX");

    CHECK(stre::trim_right("   XXX   ") == "   XXX");
    CHECK(stre::trim_right("XXX") == "XXX");

    CHECK(stre::trim("   XXX   ") == "XXX");
    CHECK(stre::trim("XXX") == "XXX");

    CHECK(stre::trim("00000010001000000", '0') == "10001");
}