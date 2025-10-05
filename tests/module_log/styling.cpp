#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Tests styling stringification output

TEST_CASE("Styling / Alignment") {
    // Left-padded values
    CHECK(log::stringify("lorem" | log::align_left(10)) == "lorem     ");
    CHECK(log::stringify("lorem" | log::align_left(2)) == "lorem");

    // Right-padded values
    CHECK(log::stringify("lorem" | log::align_right(10)) == "     lorem");
    CHECK(log::stringify("lorem" | log::align_right(2)) == "lorem");

    // Padded (center-aligned) values
    CHECK(log::stringify("lorem" | log::align_center(9)) == "  lorem  ");
    CHECK(log::stringify("lorem" | log::align_center(10)) == "  lorem   ");
    CHECK(log::stringify("lorem" | log::align_center(2)) == "lorem");
}