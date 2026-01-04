#include "tests/common.hpp"

#include "include/UTL/describe_struct.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// ========================
// --- Reflected struct ---
// ========================

struct move_only {
    std::unique_ptr<int> data;
};

UTL_DESCRIBE_STRUCT(move_only, data);

// =============
// --- Tests ---
// =============

// This tests how perfect forwarding of views and 'get()' handles r-values,
// we're checking because implementing such forwarding correctly can be tricky

TEST_CASE("Perfect forwarding") {
    move_only            object;
    std::unique_ptr<int> data;

    data = std::make_unique<int>(111);
    CHECK(*data == 111);

    object.data = std::make_unique<int>(222);
    data        = describe_struct::value<0>(std::move(object));
    CHECK(*data == 222);

    // std::unique_ptr<int> extracted_data(describe_struct::value<0>(object));
    // this will not compile because the class is move only

    // Test the same for entry view
    data = std::make_unique<int>(333);
    CHECK(*data == 333);

    object.data = std::make_unique<int>(444);
    data        = std::get<1>(std::get<0>(describe_struct::entry_view(std::move(object))));
    CHECK(*data == 444);

    // extracted_data = std::get<1>(std::get<0>(describe_struct::entry_view(object)));
    // this will not compile because the class is move only
}