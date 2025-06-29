#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Other utils") {
    CHECK(stre::repeat_char('k', 6) == "kkkkkk");
    CHECK(stre::repeat_char('k', 0) == "");

    CHECK(stre::repeat_string("xo", 3) == "xoxoxo");
    CHECK(stre::repeat_string("xo", 0) == "");

    CHECK(stre::escape_control_chars("Here is \t\n Johny!") == R"(Here is \t\n Johny!)");
    CHECK(stre::escape_control_chars("Lorem \r\r\f\a be the Ipsum!") == R"(Lorem \r\r\f\a be the Ipsum!)");
    CHECK(stre::escape_control_chars("Let this string be untouched!") == R"(Let this string be untouched!)");

    CHECK(stre::index_of_difference("0123X56789", "0123456789") == 4);
    CHECK(stre::index_of_difference("0123456789", "A123456789") == 0);
    CHECK(stre::index_of_difference("012345678G", "012345678F") == 9);
    CHECK(stre::index_of_difference("xxx", "xxx") == 3);

    CHECK_THROWS(stre::index_of_difference("xxx", "xxxx"));
}