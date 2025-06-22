// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/enum_reflect.hpp"

// _______________________ INCLUDES _______________________

// NONE

// ____________________ DEVELOPER DOCS ____________________

// Everything is constexpr so we can just use static asserts for the whole test suite

// ____________________ IMPLEMENTATION ____________________

enum class Side { LEFT = -1, RIGHT = 1, NONE = 0 };
enum class NonReflectedSide { DOWN = -2, UP = 2 };

UTL_ENUM_REFLECT(Side, LEFT, RIGHT, NONE);

TEST_CASE("Enum reflection basics work as expected") {
    static_assert(enum_reflect::type_name<Side> == "Side");

    static_assert(enum_reflect::size<Side> == 3);

    static_assert(enum_reflect::names<Side>[0] == "LEFT");
    static_assert(enum_reflect::names<Side>[1] == "RIGHT");
    static_assert(enum_reflect::names<Side>[2] == "NONE");

    static_assert(enum_reflect::values<Side>[0] == Side::LEFT);
    static_assert(enum_reflect::values<Side>[1] == Side::RIGHT);
    static_assert(enum_reflect::values<Side>[2] == Side::NONE);

    static_assert(enum_reflect::entries<Side>[0] == std::pair{"LEFT"sv, Side::LEFT});
    static_assert(enum_reflect::entries<Side>[1] == std::pair{"RIGHT"sv, Side::RIGHT});
    static_assert(enum_reflect::entries<Side>[2] == std::pair{"NONE"sv, Side::NONE});
}

TEST_CASE("Enum reflection utils work as expected") {
    static_assert(enum_reflect::is_valid(Side{-1}) == true);
    static_assert(enum_reflect::is_valid(Side{1}) == true);
    static_assert(enum_reflect::is_valid(Side{0}) == true);
    static_assert(enum_reflect::is_valid(Side{2}) == false);

    static_assert(enum_reflect::to_underlying(Side::LEFT) == -1);
    static_assert(enum_reflect::to_underlying(Side::RIGHT) == 1);
    static_assert(enum_reflect::to_underlying(Side::NONE) == 0);
}

TEST_CASE("Enum <-> string conversion works as expected") {
    static_assert(enum_reflect::to_string(Side::LEFT) == "LEFT");
    static_assert(enum_reflect::to_string(Side::RIGHT) == "RIGHT");
    static_assert(enum_reflect::to_string(Side::NONE) == "NONE");

    static_assert(enum_reflect::from_string<Side>("LEFT") == Side::LEFT);
    static_assert(enum_reflect::from_string<Side>("RIGHT") == Side::RIGHT);
    static_assert(enum_reflect::from_string<Side>("NONE") == Side::NONE);
    
    // 'throws on incorrect' tests can't be constexpr
    const bool incorrect_to_string_throws = check_if_throws([](){
        [[maybe_unused]] const auto res = enum_reflect::to_string(static_cast<Side>(17));
    });
    
    const bool incorrect_from_string_throws = check_if_throws([](){
        [[maybe_unused]] const auto res = enum_reflect::from_string<Side>("UP");
    });
    
    CHECK(incorrect_to_string_throws);
    CHECK(incorrect_from_string_throws);
}
