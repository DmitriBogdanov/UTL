#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

#include <cctype> // isalnum(), isalpha(), islower(), isupper(), isdigit(), isxdigit(), iscntrl(), isgraph(), ...

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Character classification") {
    constexpr char min = nl<char>::min();
    constexpr char max = nl<char>::max();

    // For ASCII, the library should agree with <cctype>
    for (char ch = 0; ch < max; ++ch) {
        const unsigned char uch = static_cast<unsigned char>(ch);

        CHECK(static_cast<bool>(std::isalnum(uch)) == stre::is_alphanumeric(ch));
        CHECK(static_cast<bool>(std::isalpha(uch)) == stre::is_alphabetic(ch));
        CHECK(static_cast<bool>(std::islower(uch)) == stre::is_lowercase(ch));
        CHECK(static_cast<bool>(std::isupper(uch)) == stre::is_uppercase(ch));
        CHECK(static_cast<bool>(std::isdigit(uch)) == stre::is_digit(ch));
        CHECK(static_cast<bool>(std::isxdigit(uch)) == stre::is_hexadecimal(ch));
        CHECK(static_cast<bool>(std::iscntrl(uch)) == stre::is_control(ch));
        CHECK(static_cast<bool>(std::isgraph(uch)) == stre::is_graphical(ch));
        CHECK(static_cast<bool>(std::isspace(uch)) == stre::is_space(ch));
        CHECK(static_cast<bool>(std::isblank(uch)) == stre::is_blank(ch));
        CHECK(static_cast<bool>(std::isprint(uch)) == stre::is_printable(ch));
        CHECK(static_cast<bool>(std::ispunct(uch)) == stre::is_punctuation(ch));
        // Note: <cctype> functions don't return just 0 or 1, they can
        //       return any non-zero values so we have to cast explicitly
    }
    // For other (non-ASCII) values we should get 'false'
    for (char ch = min; ch < 0; ++ch) {
        CHECK(!stre::is_alphanumeric(ch));
        CHECK(!stre::is_alphabetic(ch));
        CHECK(!stre::is_lowercase(ch));
        CHECK(!stre::is_uppercase(ch));
        CHECK(!stre::is_digit(ch));
        CHECK(!stre::is_hexadecimal(ch));
        CHECK(!stre::is_control(ch));
        CHECK(!stre::is_graphical(ch));
        CHECK(!stre::is_space(ch));
        CHECK(!stre::is_blank(ch));
        CHECK(!stre::is_printable(ch));
        CHECK(!stre::is_punctuation(ch));
    }
}