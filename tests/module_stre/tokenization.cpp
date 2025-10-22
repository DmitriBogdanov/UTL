#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Tokenization / Tokenize") { // 'tokenize()' ignores empty tokens

    { // simplest case
        const auto tokens = stre::tokenize("aaa,bbb,ccc", ",");
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "aaa");
        CHECK(tokens[1] == "bbb");
        CHECK(tokens[2] == "ccc");
    }

    { // leading delimiters
        const auto tokens = stre::tokenize("(---)lorem(---)ipsum", "(---)");
        CHECK(tokens.size() == 2);
        CHECK(tokens[0] == "lorem");
        CHECK(tokens[1] == "ipsum");
    }

    { // leading + repeating delimiters
        const auto tokens = stre::tokenize("___lorem_________ipsum", "___");
        CHECK(tokens.size() == 2);
        CHECK(tokens[0] == "lorem");
        CHECK(tokens[1] == "ipsum");
    }

    { // leading + repeating + trailing delimiters
        const auto tokens = stre::tokenize("xxAxxxxxBxCxDxxEx", "x");
        CHECK(tokens.size() == 5);
        CHECK(tokens[0] == "A");
        CHECK(tokens[1] == "B");
        CHECK(tokens[2] == "C");
        CHECK(tokens[3] == "D");
        CHECK(tokens[4] == "E");
    }

    { // non-empty string with a single delimiter-like token
        const auto tokens = stre::tokenize(",,", ",,,");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == ",,");
    }

    { // non-empty string with no tokens
        const auto tokens = stre::tokenize(".........", "...");
        CHECK(tokens.size() == 0);
    }

    { // empty string with no tokens
        const auto tokens = stre::tokenize("", "...");
        CHECK(tokens.size() == 0);
    }

    { // non-empty string with empty delimiter
        const auto tokens = stre::tokenize("text", "");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "text");
    }
}

TEST_CASE("Tokenization / Split") { // 'split()' preserves empty tokens

    { // simplest case
        const auto tokens = stre::split("aaa,bbb,ccc", ",");
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "aaa");
        CHECK(tokens[1] == "bbb");
        CHECK(tokens[2] == "ccc");
    }

    { // leading delimiters
        const auto tokens = stre::split("(---)lorem(---)ipsum", "(---)");
        CHECK(tokens.size() == 3);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "lorem");
        CHECK(tokens[2] == "ipsum");
    }

    { // leading + repeating delimiters
        const auto tokens = stre::split("___lorem_________ipsum", "___");
        CHECK(tokens.size() == 5);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "lorem");
        CHECK(tokens[2] == "");
        CHECK(tokens[3] == "");
        CHECK(tokens[4] == "ipsum");
    }

    { // leading + repeating + trailing delimiters
        const auto tokens = stre::split("xxAxxxxxBxCxDxxEx", "x");
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
        const auto tokens = stre::split(",,", ",,,");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == ",,");
    }

    { // non-empty string with no tokens
        const auto tokens = stre::split(".........", "...");
        CHECK(tokens.size() == 4);
        CHECK(tokens[0] == "");
        CHECK(tokens[1] == "");
        CHECK(tokens[2] == "");
        CHECK(tokens[3] == "");
    }

    { // empty string with no tokens
        const auto tokens = stre::split("", "...");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "");
    }

    { // non-empty string with empty delimiter
        const auto tokens = stre::split("text", "");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0] == "text");
    }
}