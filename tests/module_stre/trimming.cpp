#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Trimming / std::string_view") {
    using namespace std::string_view_literals;

    static_assert(stre::trim_left("   XXX   "sv) == "XXX   "); // trim something
    static_assert(stre::trim_left("XXX   "sv) == "XXX   ");    // trim nothing
    static_assert(stre::trim_left("   "sv) == "");             // trim everything

    static_assert(stre::trim_left("sssXXXsss"sv, 's') == "XXXsss"); // trim something
    static_assert(stre::trim_left("XXXsss"sv, 's') == "XXXsss");    // trim nothing
    static_assert(stre::trim_left("sss"sv, 's') == "");             // trim everything

    static_assert(stre::trim_right("   XXX   "sv) == "   XXX"); // trim something
    static_assert(stre::trim_right("   XXX"sv) == "   XXX");    // trim nothing
    static_assert(stre::trim_right("   "sv) == "");             // trim everything

    static_assert(stre::trim_right("sssXXXsss"sv, 's') == "sssXXX"); // trim something
    static_assert(stre::trim_right("sssXXX"sv, 's') == "sssXXX");    // trim nothing
    static_assert(stre::trim_right("sss"sv, 's') == "");             // trim everything

    static_assert(stre::trim("   XXX   "sv) == "XXX"); // trim both sides
    static_assert(stre::trim("   XXX"sv) == "XXX");    // trim left only
    static_assert(stre::trim("XXX   "sv) == "XXX");    // trim right only
    static_assert(stre::trim("XXX"sv) == "XXX");       // trim nothing
    static_assert(stre::trim("   "sv) == "");          // trim everything

    static_assert(stre::trim("sssXXXsss"sv, 's') == "XXX"); // trim both sides
    static_assert(stre::trim("sssXXX"sv, 's') == "XXX");    // trim left only
    static_assert(stre::trim("XXXsss"sv, 's') == "XXX");    // trim right only
    static_assert(stre::trim("XXX"sv, 's') == "XXX");       // trim nothing
    static_assert(stre::trim("sss"sv, 's') == "");          // trim everything
}

TEST_CASE("Trimming / C-string") {
    static_assert(stre::trim_left("   XXX   ") == "XXX   "); // trim something
    static_assert(stre::trim_left("XXX   ") == "XXX   ");    // trim nothing
    static_assert(stre::trim_left("   ") == "");             // trim everything
    
    static_assert(stre::trim_left("sssXXXsss", 's') == "XXXsss"); // trim something
    static_assert(stre::trim_left("XXXsss", 's') == "XXXsss");    // trim nothing
    static_assert(stre::trim_left("sss", 's') == "");             // trim everything
    
    static_assert(stre::trim_right("   XXX   ") == "   XXX"); // trim something
    static_assert(stre::trim_right("   XXX") == "   XXX");    // trim nothing
    static_assert(stre::trim_right("   ") == "");             // trim everything
    
    static_assert(stre::trim_right("sssXXXsss", 's') == "sssXXX"); // trim something
    static_assert(stre::trim_right("sssXXX", 's') == "sssXXX");    // trim nothing
    static_assert(stre::trim_right("sss", 's') == "");             // trim everything
    
    static_assert(stre::trim("   XXX   ") == "XXX"); // trim both sides
    static_assert(stre::trim("   XXX") == "XXX");    // trim left only
    static_assert(stre::trim("XXX   ") == "XXX");    // trim right only
    static_assert(stre::trim("XXX") == "XXX");       // trim nothing
    static_assert(stre::trim("   ") == "");          // trim everything
    
    static_assert(stre::trim("sssXXXsss", 's') == "XXX"); // trim both sides
    static_assert(stre::trim("sssXXX", 's') == "XXX");    // trim left only
    static_assert(stre::trim("XXXsss", 's') == "XXX");    // trim right only
    static_assert(stre::trim("XXX", 's') == "XXX");       // trim nothing
    static_assert(stre::trim("sss", 's') == "");          // trim everything
}

TEST_CASE("Trimming / std::string") {
    using namespace std::string_literals;

    CHECK(stre::trim_left("   XXX   "s) == "XXX   "); // trim something
    CHECK(stre::trim_left("XXX   "s) == "XXX   ");    // trim nothing
    CHECK(stre::trim_left("   "s) == "");             // trim everything

    CHECK(stre::trim_left("sssXXXsss"s, 's') == "XXXsss"); // trim something
    CHECK(stre::trim_left("XXXsss"s, 's') == "XXXsss");    // trim nothing
    CHECK(stre::trim_left("sss"s, 's') == "");             // trim everything

    CHECK(stre::trim_right("   XXX   "s) == "   XXX"); // trim something
    CHECK(stre::trim_right("   XXX"s) == "   XXX");    // trim nothing
    CHECK(stre::trim_right("   "s) == "");             // trim everything

    CHECK(stre::trim_right("sssXXXsss"s, 's') == "sssXXX"); // trim something
    CHECK(stre::trim_right("sssXXX"s, 's') == "sssXXX");    // trim nothing
    CHECK(stre::trim_right("sss"s, 's') == "");             // trim everything

    CHECK(stre::trim("   XXX   "s) == "XXX"); // trim both sides
    CHECK(stre::trim("   XXX"s) == "XXX");    // trim left only
    CHECK(stre::trim("XXX   "s) == "XXX");    // trim right only
    CHECK(stre::trim("XXX"s) == "XXX");       // trim nothing
    CHECK(stre::trim("   "s) == "");          // trim everything

    CHECK(stre::trim("sssXXXsss"s, 's') == "XXX"); // trim both sides
    CHECK(stre::trim("sssXXX"s, 's') == "XXX");    // trim left only
    CHECK(stre::trim("XXXsss"s, 's') == "XXX");    // trim right only
    CHECK(stre::trim("XXX"s, 's') == "XXX");       // trim nothing
    CHECK(stre::trim("sss"s, 's') == "");          // trim everything
}