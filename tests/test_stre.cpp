// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "test.hpp"

#include "UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ========================================
// --- Tests for "simple" functionality ---
// ========================================

TEST_CASE("Formatting utils perform as expected") {

    CHECK(stre::pad_with_leading_zeroes(15, 4) == "0015");
    CHECK(stre::pad_with_leading_zeroes(174, 10) == "0000000174");
    CHECK(stre::pad_with_leading_zeroes(137, 3) == "137");
    CHECK(stre::pad_with_leading_zeroes(4321, 2) == "4321");

    CHECK(stre::repeat_char('k', 6) == "kkkkkk");
    CHECK(stre::repeat_char('k', 0) == "");

    CHECK(stre::repeat_string("xo", 3) == "xoxoxo");
    CHECK(stre::repeat_string("xo", 0) == "");
}

TEST_CASE("Trimming") {
    CHECK(stre::trim_left("   XXX   ") == "XXX   ");
    CHECK(stre::trim_left("XXX") == "XXX");

    CHECK(stre::trim_right("   XXX   ") == "   XXX");
    CHECK(stre::trim_right("XXX") == "XXX");

    CHECK(stre::trim("   XXX   ") == "XXX");
    CHECK(stre::trim("XXX") == "XXX");

    CHECK(stre::trim("00000010001000000", '0') == "10001");
}

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

TEST_CASE("Case conversions") {
    CHECK(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
    CHECK(stre::to_lower("XXX") == "xxx");
    CHECK(stre::to_lower("Some \t\n\r VERY \17 StRanGe text -=14") == "some \t\n\r very \17 strange text -=14");

    CHECK(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
    CHECK(stre::to_upper("xxx") == "XXX");
    CHECK(stre::to_upper("some \t\n\r very \17 strange text -=14") == "SOME \t\n\r VERY \17 STRANGE TEXT -=14");
}

TEST_CASE("Substring checks") {
    CHECK(stre::starts_with("Lorem Ipsum", "Lorem"));
    CHECK(!stre::starts_with("Lorem Ipsum", "Ipsum"));

    CHECK(stre::ends_with("Lorem Ipsum", "Ipsum"));
    CHECK(!stre::ends_with("Lorem Ipsum", "Lorem"));

    CHECK(stre::contains("Some \t\r\n rather 17 bizarre TeXt", "\t\r\n"));
    CHECK(stre::contains("Some \t\r\n rather 17 bizarre TeXt", " bizarre TeXt"));
    CHECK(!stre::contains("Some \t\r\n rather 17 bizarre TeXt", "15"));
}

// =========================================
// --- Tests for "complex" functionality ---
// =========================================

TEST_CASE("Splitting string by delimiter (keep_empty_tokens = false)") {
    { // simplest case
        const auto tokens = stre::split_by_delimiter("aaa,bbb,ccc", ",");
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "aaa");
        CHECK(tokens[1] == "bbb");
        CHECK(tokens[2] == "ccc");
    }

    { // leading delimiters
        const auto tokens = stre::split_by_delimiter("(---)lorem(---)ipsum", "(---)");
        CHECK(tokens.size() == 2);
        CHECK(tokens[0] == "lorem");
        CHECK(tokens[1] == "ipsum");
    }

    { // leading + repeating delimiters
        const auto tokens = stre::split_by_delimiter("___lorem_________ipsum", "___");
        CHECK(tokens.size() == 2);
        CHECK(tokens[0] == "lorem");
        CHECK(tokens[1] == "ipsum");
    }

    { // leading + repeating + trailing delimiters
        const auto tokens = stre::split_by_delimiter("xxAxxxxxBxCxDxxEx", "x");
        CHECK(tokens.size() == 5);
        CHECK(tokens[0] == "A");
        CHECK(tokens[1] == "B");
        CHECK(tokens[2] == "C");
        CHECK(tokens[3] == "D");
        CHECK(tokens[4] == "E");
    }

    { // non-empty string with a single delimiter-like token
        const auto tokens = stre::split_by_delimiter(",,", ",,,");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == ",,");
    }

    { // non-empty string with no tokens
        const auto tokens = stre::split_by_delimiter(".........", "...");
        CHECK(tokens.size() == 0);
    }

    { // empty string with no tokens
        const auto tokens = stre::split_by_delimiter("", "...");
        CHECK(tokens.size() == 0);
    }

    { // non-empty string with empty delimiter
        const auto tokens = stre::split_by_delimiter("text", "");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "text");
    }
}

TEST_CASE("Splitting string by delimiter (keep_empty_tokens = true)") {
    { // simplest case
        const auto tokens = stre::split_by_delimiter("aaa,bbb,ccc", ",", true);
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "aaa");
        CHECK(tokens[1] == "bbb");
        CHECK(tokens[2] == "ccc");
    }

    { // leading delimiters
        const auto tokens = stre::split_by_delimiter("(---)lorem(---)ipsum", "(---)", true);
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "lorem");
        CHECK(tokens[2] == "ipsum");
    }

    { // leading + repeating delimiters
        const auto tokens = stre::split_by_delimiter("___lorem_________ipsum", "___", true);
        CHECK(tokens.size() == 5);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "lorem");
        CHECK(tokens[2] == "");
        CHECK(tokens[3] == "");
        CHECK(tokens[4] == "ipsum");
    }

    { // leading + repeating + trailing delimiters
        const auto tokens = stre::split_by_delimiter("xxAxxxxxBxCxDxxEx", "x", true);
        CHECK(tokens.size() == 13);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "");
        CHECK(tokens[2] == "A");
        CHECK(tokens[3] == "");
        CHECK(tokens[4] == "");
        CHECK(tokens[5] == "");
        CHECK(tokens[6] == "");
        CHECK(tokens[7] == "B");
        CHECK(tokens[8] == "C");
        CHECK(tokens[9] == "D");
        CHECK(tokens[10] == "");
        CHECK(tokens[11] == "E");
        CHECK(tokens[12] == "");
    }

    { // non-empty string with a single delimiter-like token
        const auto tokens = stre::split_by_delimiter(",,", ",,,", true);
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == ",,");
    }

    { // non-empty string with no tokens
        const auto tokens = stre::split_by_delimiter(".........", "...", true);
        CHECK(tokens.size() == 4);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "");
        CHECK(tokens[2] == "");
        CHECK(tokens[3] == "");
    }

    { // empty string with no tokens
        const auto tokens = stre::split_by_delimiter("", "...", true);
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "");
    }

    { // non-empty string with empty delimiter
        const auto tokens = stre::split_by_delimiter("text", "", true);
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "text");
    }
}

TEST_CASE("Other utils") {
    CHECK(stre::replace_all_occurrences("xxxAAxxxAAxxx", "AA", "BBB") == "xxxBBBxxxBBBxxx");
    CHECK(stre::replace_all_occurrences("Some very very cool text ending with very", "very", "really") ==
          "Some really really cool text ending with really");
    CHECK(stre::replace_all_occurrences("very short string", "super long replacement target",
                                       "even longer replacement string just to test thing out") == "very short string");

    CHECK(stre::escape_control_chars("Here is \t\n Johny!") == R"(Here is \t\n Johny!)");
    CHECK(stre::escape_control_chars("Lorem \r\r\f\a be the Ipsum!") == R"(Lorem \r\r\f\a be the Ipsum!)");
    CHECK(stre::escape_control_chars("Let this string be untouched!") == R"(Let this string be untouched!)");

    CHECK(stre::index_of_difference("0123X56789", "0123456789") == 4);
    CHECK(stre::index_of_difference("0123456789", "A123456789") == 0);
    CHECK(stre::index_of_difference("012345678G", "012345678F") == 9);
    CHECK(stre::index_of_difference("xxx", "xxx") == 3);

    const bool incompatible_sizes_throw = check_if_throws([]() { return stre::index_of_difference("xxx", "xxxx"); });
    CHECK(incompatible_sizes_throw);
}