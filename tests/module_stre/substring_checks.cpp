#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Substring checks") {
    CHECK(stre::starts_with("Lorem Ipsum", "Lorem"));
    CHECK(!stre::starts_with("Lorem Ipsum", "Ipsum"));

    CHECK(stre::ends_with("Lorem Ipsum", "Ipsum"));
    CHECK(!stre::ends_with("Lorem Ipsum", "Lorem"));

    CHECK(stre::contains("Some \t\r\n rather 17 bizarre TeXt", "\t\r\n"));
    CHECK(stre::contains("Some \t\r\n rather 17 bizarre TeXt", " bizarre TeXt"));
    CHECK(!stre::contains("Some \t\r\n rather 17 bizarre TeXt", "15"));
}