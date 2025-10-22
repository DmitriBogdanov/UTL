#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Other / Difference measurement") {
    static_assert(stre::first_difference("0123X56789", "0123456789") == 4);
    static_assert(stre::first_difference("0123456789", "A123456789") == 0);
    static_assert(stre::first_difference("012345678G", "012345678F") == 9);
    static_assert(stre::first_difference("xxx", "xxxxxx") == 3);
    static_assert(stre::first_difference("yyyy", "yy") == 2);
    static_assert(stre::first_difference("xxx", "xxx") == std::string_view::npos);
    
    static_assert(stre::count_difference("0123X56789", "0123456789") == 1);
    static_assert(stre::count_difference("0123456789", "A123456789") == 1);
    static_assert(stre::count_difference("012345678G", "012345678F") == 1);
    static_assert(stre::count_difference("xxx", "xxxxxx") == 3);
    static_assert(stre::count_difference("yyyy", "yy") == 2);
    static_assert(stre::count_difference("xxx", "xxx") == 0);
}