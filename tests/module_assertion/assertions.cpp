#include "tests/common.hpp"

#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

// _______________________ INCLUDES _______________________

#include <array>     // array<>
#include <memory>    // make_unique()
#include <stdexcept> // runtime_error

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Assertions") {
    // Make assertions throw instead of aborting so we can test them
    assertion::set_handler([](const assertion::FailureInfo& info) { throw std::runtime_error(info.to_string()); });

    // Test all the decomposed binary operators
    CHECK_NOTHROW(ASSERT(2 + 1 < 4));
    CHECK_NOTHROW(ASSERT(2 + 1 <= 4));
    CHECK_NOTHROW(ASSERT(2 + 2 <= 4));
    CHECK_NOTHROW(ASSERT(2 + 2 == 4));
    CHECK_NOTHROW(ASSERT(2 + 2 != 5));
    CHECK_NOTHROW(ASSERT(2 + 2 >= 4));
    CHECK_NOTHROW(ASSERT(2 + 3 >= 4));
    CHECK_NOTHROW(ASSERT(2 + 3 > 4));

    CHECK_THROWS(ASSERT(2 + 1 > 4));
    CHECK_THROWS(ASSERT(2 + 2 > 4));
    CHECK_THROWS(ASSERT(2 + 1 >= 4));
    CHECK_THROWS(ASSERT(2 + 2 != 4));
    CHECK_THROWS(ASSERT(2 + 2 == 5));
    CHECK_THROWS(ASSERT(2 + 3 <= 4));
    CHECK_THROWS(ASSERT(2 + 2 < 4));
    CHECK_THROWS(ASSERT(2 + 3 < 4));

    // Test various unary assertions
    const auto array = std::array{1, 2, 3};
    const auto ptr   = std::make_unique<int>(1);

    CHECK_NOTHROW(ASSERT(true));
    CHECK_NOTHROW(ASSERT(1));
    CHECK_NOTHROW(ASSERT(-1));
    CHECK_NOTHROW(ASSERT(std::size_t(15)));
    CHECK_NOTHROW(ASSERT(array.data()));
    CHECK_NOTHROW(ASSERT(array.size()));
    CHECK_NOTHROW(ASSERT(ptr.get()));

    CHECK_THROWS(ASSERT(false));
    CHECK_THROWS(ASSERT(0));
    CHECK_THROWS(ASSERT(5 - 5));
    CHECK_THROWS(ASSERT(std::size_t(0)));
    CHECK_THROWS(ASSERT(nullptr));
}