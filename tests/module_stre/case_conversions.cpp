#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Case conversions") {
    static_assert(stre::to_lower('L') == 'l');
    static_assert(stre::to_lower('l') == 'l');
    static_assert(stre::to_lower('\n') == '\n');
    static_assert(stre::to_lower('5') == '5');
    
    static_assert(stre::to_upper('L') == 'L');
    static_assert(stre::to_upper('l') == 'L');
    static_assert(stre::to_upper('\n') == '\n');
    static_assert(stre::to_upper('5') == '5');
    
    CHECK(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
    CHECK(stre::to_lower("XXX") == "xxx");
    CHECK(stre::to_lower("Some \t\n\r VERY \17 StRanGe text -=14") == "some \t\n\r very \17 strange text -=14");

    CHECK(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
    CHECK(stre::to_upper("xxx") == "XXX");
    CHECK(stre::to_upper("some \t\n\r very \17 strange text -=14") == "SOME \t\n\r VERY \17 STRANGE TEXT -=14");
}