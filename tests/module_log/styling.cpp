#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Tests styling stringification output

TEST_CASE("Styling / Alignment") {
    // Left-padded values
    CHECK(log::stringify(log::PadLeft{"lorem", 10}) == "     lorem");
    CHECK(log::stringify(log::PadLeft{"lorem", 2}) == "lorem");

    // Right-padded values
    CHECK(log::stringify(log::PadRight{"lorem", 10}) == "lorem     ");
    CHECK(log::stringify(log::PadRight{"lorem", 2}) == "lorem");

    // Padded (center-aligned) values
    CHECK(log::stringify(log::Pad{"lorem", 9}) == "  lorem  ");
    CHECK(log::stringify(log::Pad{"lorem", 10}) == "  lorem   ");
    CHECK(log::stringify(log::Pad{"lorem", 2}) == "lorem");
}