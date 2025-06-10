// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "test.hpp"

#include "UTL/struct_reflect.hpp"

// _______________________ INCLUDES _______________________

#include <algorithm>
#include <memory>

// ____________________ DEVELOPER DOCS ____________________

// Everything is constexpr so we can just use static asserts for the whole test suite

// ____________________ IMPLEMENTATION ____________________

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

TEST_CASE("Struct reflection basics work as expected") {
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

TEST_CASE("Struct reflection binary operators work as expected") {
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} == Quaternion{6, 8, 10, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{7, 8, 10, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{6, 9, 11, 12});
    static_assert(Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} != Quaternion{6, 8, 10, 13});
}

struct MoveOnlyClass {
    std::unique_ptr<int> data;
};

UTL_STRUCT_REFLECT(MoveOnlyClass, data);

TEST_CASE("Struct reflection tuple view perfect forwarding works as expected") {
    MoveOnlyClass move_only;
    std::unique_ptr<int> extracted_data;
    
    extracted_data = std::make_unique<int>(111);
    CHECK(*extracted_data == 111);
    
    move_only.data = std::make_unique<int>(222);
    extracted_data = struct_reflect::get<0>(std::move(move_only));
    CHECK(*extracted_data == 222);
    
    // std::unique_ptr<int> extracted_data(struct_reflect::get<0>(move_only));
    // this will not compile because the class is move only
  
    // Test the same for entry view
    extracted_data = std::make_unique<int>(333);
    CHECK(*extracted_data == 333);
    
    move_only.data = std::make_unique<int>(444);
    extracted_data = std::get<1>(std::get<0>(struct_reflect::entry_view(std::move(move_only))));
    CHECK(*extracted_data == 444);
    
    // extracted_data = std::get<1>(std::get<0>(struct_reflect::entry_view(move_only)));
    // this will not compile because the class is move only
}