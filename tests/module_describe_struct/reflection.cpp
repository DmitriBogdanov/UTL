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
    return describe_struct::true_for_all(lhs, rhs, [&](const auto& l, const auto& r) { return l == r; });
}

constexpr bool operator!=(const quaternion& lhs, const quaternion& rhs) { return !(lhs == rhs); }

constexpr quaternion operator+(const quaternion& lhs, const quaternion& rhs) {
    quaternion res = lhs;
    describe_struct::for_each(res, rhs, [&](auto& l, const auto& r) { l += r; });
    return res;
}

// =============
// --- Tests ---
// =============

TEST_CASE("Reflection / Core") {
    static_assert(describe_struct::type_name<quaternion> == "quaternion");

    static_assert(describe_struct::size<quaternion> == 4);

    static_assert(describe_struct::names<quaternion>[0] == "r");
    static_assert(describe_struct::names<quaternion>[1] == "i");
    static_assert(describe_struct::names<quaternion>[2] == "j");
    static_assert(describe_struct::names<quaternion>[3] == "k");

    constexpr quaternion q = {5., 6., 7., 8.};

    static_assert(describe_struct::get<0>(q) == 5.);
    static_assert(describe_struct::get<1>(q) == 6.);
    static_assert(describe_struct::get<2>(q) == 7.);
    static_assert(describe_struct::get<3>(q) == 8.);
}

TEST_CASE("Reflection / Views") {
    constexpr quaternion q = {5., 6., 7., 8.};

    static_assert(describe_struct::field_view(q) == std::tuple{5., 6., 7., 8.});

    static_assert(std::get<0>(describe_struct::entry_view(q)).first == "r");
    static_assert(std::get<0>(describe_struct::entry_view(q)).second == 5.);
    static_assert(std::get<1>(describe_struct::entry_view(q)).first == "i");
    static_assert(std::get<1>(describe_struct::entry_view(q)).second == 6.);
    static_assert(std::get<2>(describe_struct::entry_view(q)).first == "j");
    static_assert(std::get<2>(describe_struct::entry_view(q)).second == 7.);
    static_assert(std::get<3>(describe_struct::entry_view(q)).first == "k");
    static_assert(std::get<3>(describe_struct::entry_view(q)).second == 8.);
}

TEST_CASE("Reflection / Loops and predicates") {
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} == quaternion{6, 8, 10, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{7, 8, 10, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{6, 9, 11, 12});
    static_assert(quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} != quaternion{6, 8, 10, 13});
}