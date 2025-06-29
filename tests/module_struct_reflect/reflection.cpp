#include "tests/common.hpp"

#include "include/UTL/struct_reflect.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// ========================
// --- Reflected struct ---
// ========================

struct Quaternion {
    double r, i, j, k;
};

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs) {
    return struct_reflect::true_for_all(lhs, rhs, [&](const auto& l, const auto& r) { return l == r; });
}

constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs) { return !(lhs == rhs); }

constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs) {
    Quaternion res = lhs;
    struct_reflect::for_each(res, rhs, [&](auto& l, const auto& r) { l += r; });
    return res;
}

// =============
// --- Tests ---
// =============

TEST_CASE("Reflection / Core") {
    static_assert(struct_reflect::type_name<Quaternion> == "Quaternion");

    static_assert(struct_reflect::size<Quaternion> == 4);

    static_assert(struct_reflect::names<Quaternion>[0] == "r");
    static_assert(struct_reflect::names<Quaternion>[1] == "i");
    static_assert(struct_reflect::names<Quaternion>[2] == "j");
    static_assert(struct_reflect::names<Quaternion>[3] == "k");

    constexpr Quaternion q = {5., 6., 7., 8.};

    static_assert(struct_reflect::get<0>(q) == 5.);
    static_assert(struct_reflect::get<1>(q) == 6.);
    static_assert(struct_reflect::get<2>(q) == 7.);
    static_assert(struct_reflect::get<3>(q) == 8.);
}

TEST_CASE("Reflection / Views") {
    constexpr Quaternion q = {5., 6., 7., 8.};
    
    static_assert(struct_reflect::field_view(q) == std::tuple{5., 6., 7., 8.});

    static_assert(std::get<0>(struct_reflect::entry_view(q)).first == "r");
    static_assert(std::get<0>(struct_reflect::entry_view(q)).second == 5.);
    static_assert(std::get<1>(struct_reflect::entry_view(q)).first == "i");
    static_assert(std::get<1>(struct_reflect::entry_view(q)).second == 6.);
    static_assert(std::get<2>(struct_reflect::entry_view(q)).first == "j");
    static_assert(std::get<2>(struct_reflect::entry_view(q)).second == 7.);
    static_assert(std::get<3>(struct_reflect::entry_view(q)).first == "k");
    static_assert(std::get<3>(struct_reflect::entry_view(q)).second == 8.);
}

TEST_CASE("Reflection / Loops and predicates") {
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} == Quaternion{6, 8, 10, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{7, 8, 10, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{6, 9, 11, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{6, 8, 10, 13});
}