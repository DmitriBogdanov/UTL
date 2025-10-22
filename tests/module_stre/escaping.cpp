#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Escaping") {
    CHECK(stre::escape('0') == R"(0)");
    CHECK(stre::escape('A') == R"(A)");
    CHECK(stre::escape('_') == R"(_)");
    CHECK(stre::escape('\n') == R"(\n)");
    CHECK(stre::escape('\t') == R"(\t)");
    CHECK(stre::escape('\r') == R"(\r)");
    CHECK(stre::escape('\f') == R"(\f)");
    CHECK(stre::escape('\a') == R"(\a)");
    CHECK(stre::escape('\b') == R"(\b)");
    CHECK(stre::escape('\v') == R"(\v)");
    
    CHECK(stre::escape("Here is \t\n Johny!") == R"(Here is \t\n Johny!)");
    CHECK(stre::escape("Lorem \r\r\f\a be the Ipsum!") == R"(Lorem \r\r\f\a be the Ipsum!)");
    CHECK(stre::escape("Let this string be untouched!") == R"(Let this string be untouched!)");
    CHECK(stre::escape("\n\n\n\n\n\n") == R"(\n\n\n\n\n\n)");
    CHECK(stre::escape("\n\t\r\f\a\b\v") == R"(\n\t\r\f\a\b\v)");
    CHECK(stre::escape("\nA\tB\rC\fD\aE\bF\v") == R"(\nA\tB\rC\fD\aE\bF\v)");
}