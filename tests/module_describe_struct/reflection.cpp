#include "tests/common.hpp"

#include "include/UTL/describe_struct.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// ========================
// --- Reflected struct ---
// ========================

struct quaternion {
    double r, i, j, k;
};

UTL_DESCRIBE_STRUCT(quaternion, r, i, j, k);

constexpr bool operator==(const quaternion& lhs, const quaternion& rhs) {
    bool res = true;

    describe_struct::for_indices<quaternion>([&](auto i) {
        if (describe_struct::value<i>(lhs) != describe_struct::value<i>(rhs)) res = false;
    });

    return res;
}

constexpr bool operator!=(const quaternion& lhs, const quaternion& rhs) { return !(lhs == rhs); }

constexpr quaternion operator+(const quaternion& lhs, const quaternion& rhs) {
    quaternion res = lhs;

    describe_struct::for_indices<quaternion>(
        [&](auto i) { describe_struct::value<i>(res) += describe_struct::value<i>(rhs); });

    return res;
}

// =============
// --- Tests ---
// =============

TEST_CASE("Reflection / General reflection") {
    static_assert(describe_struct::name<quaternion> == "quaternion");

    static_assert(describe_struct::size<quaternion> == 4);
}

TEST_CASE("Reflection / Member reflection") {
    constexpr quaternion q = {5., 6., 7., 8.};

    static_assert(describe_struct::label<0, quaternion>() == "r");
    static_assert(describe_struct::label<1, quaternion>() == "i");
    static_assert(describe_struct::label<2, quaternion>() == "j");
    static_assert(describe_struct::label<3, quaternion>() == "k");

    static_assert(describe_struct::label<0>(q) == "r");
    static_assert(describe_struct::label<1>(q) == "i");
    static_assert(describe_struct::label<2>(q) == "j");
    static_assert(describe_struct::label<3>(q) == "k");

    static_assert(describe_struct::value<0>(q) == 5.);
    static_assert(describe_struct::value<1>(q) == 6.);
    static_assert(describe_struct::value<2>(q) == 7.);
    static_assert(describe_struct::value<3>(q) == 8.);

    static_assert(describe_struct::entry<0>(q).first == "r");
    static_assert(describe_struct::entry<1>(q).first == "i");
    static_assert(describe_struct::entry<2>(q).first == "j");
    static_assert(describe_struct::entry<3>(q).first == "k");

    static_assert(describe_struct::entry<0>(q).second == 5.);
    static_assert(describe_struct::entry<1>(q).second == 6.);
    static_assert(describe_struct::entry<2>(q).second == 7.);
    static_assert(describe_struct::entry<3>(q).second == 8.);
}

TEST_CASE("Reflection / Tuple API") {
    constexpr quaternion q = {5., 6., 7., 8.};

    static_assert(std::get<0>(describe_struct::label_view(q)) == "r"); // tuple access
    static_assert(std::get<1>(describe_struct::label_view(q)) == "i");
    static_assert(std::get<2>(describe_struct::label_view(q)) == "j");
    static_assert(std::get<3>(describe_struct::label_view(q)) == "k");

    static_assert(describe_struct::label_view(q).at(0) == "r"); // array access
    static_assert(describe_struct::label_view(q).at(1) == "i");
    static_assert(describe_struct::label_view(q).at(2) == "j");
    static_assert(describe_struct::label_view(q).at(3) == "k");

    static_assert(describe_struct::value_view(q) == std::tuple{5., 6., 7., 8.});

    static_assert(std::get<0>(describe_struct::entry_view(q)).first == "r");
    static_assert(std::get<1>(describe_struct::entry_view(q)).first == "i");
    static_assert(std::get<2>(describe_struct::entry_view(q)).first == "j");
    static_assert(std::get<3>(describe_struct::entry_view(q)).first == "k");

    static_assert(std::get<0>(describe_struct::entry_view(q)).second == 5.);
    static_assert(std::get<1>(describe_struct::entry_view(q)).second == 6.);
    static_assert(std::get<2>(describe_struct::entry_view(q)).second == 7.);
    static_assert(std::get<3>(describe_struct::entry_view(q)).second == 8.);
}

TEST_CASE("Reflection / Algorithms") {
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} == quaternion{6, 8, 10, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{7, 8, 10, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{6, 9, 11, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{6, 8, 10, 13});
}