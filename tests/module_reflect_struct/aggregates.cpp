#include "tests/common.hpp"

#include "include/UTL/reflect_struct.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Resolve defect report LWG 3865 ('std::pair' should accept heterogenous comparison)
// unless the compiler already implements a fix (which we can detect with a concept)
template <class T1, class T2, class U1, class U2>
    requires ( !requires { { std::pair<T1, T2>{} != std::pair<U1, U2>{} } -> std::same_as<bool>; } )
constexpr bool operator==(const std::pair<T1, T2>& lhs, const std::pair<U1, U2>& rhs) {
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

struct vector {
    double x, y, z;
};

struct global_aggregate {};

struct outer_aggregate {
    struct inner_aggregate {};
};

namespace space {
struct namespace_aggregate {};
} // namespace space

namespace {
struct anonymous_namespace_aggregate {};
}; // namespace

using anonymous_type_aggregate = struct {};

struct derived_aggregate : global_aggregate {};

struct non_aggregate { non_aggregate(int) {} }; // non-trivial constructor => struct not an aggregate

TEST_CASE("Concepts") {
    static_assert(reflect_struct::reflectable<global_aggregate>);
    static_assert(reflect_struct::reflectable<outer_aggregate::inner_aggregate>);
    static_assert(reflect_struct::reflectable<space::namespace_aggregate>);
    static_assert(reflect_struct::reflectable<anonymous_namespace_aggregate>);
    static_assert(reflect_struct::reflectable<anonymous_type_aggregate>);
    
    static_assert(!reflect_struct::reflectable<derived_aggregate>);
    static_assert(!reflect_struct::reflectable<non_aggregate>);
}

TEST_CASE("Name reflection") {
    static_assert(reflect_struct::name<global_aggregate> == "global_aggregate");
    static_assert(reflect_struct::name<outer_aggregate::inner_aggregate> == "outer_aggregate::inner_aggregate");
    static_assert(reflect_struct::name<space::namespace_aggregate> == "space::namespace_aggregate");
    
    // Note: Cannot give any guarantees about anonymous namespaces & types on different toolchains
    
    static_assert(reflect_struct::size<global_aggregate> == 0);
    static_assert(reflect_struct::size<outer_aggregate::inner_aggregate> == 0);
    static_assert(reflect_struct::size<space::namespace_aggregate> == 0);
    static_assert(reflect_struct::size<anonymous_namespace_aggregate> == 0);
    static_assert(reflect_struct::size<anonymous_type_aggregate> == 0);
}

TEST_CASE("Basic reflection") {
    static_assert(reflect_struct::name<vector> == "vector");
    static_assert(reflect_struct::size<vector> == 3);

    constexpr vector vec = {1., 2., 3.};

    static_assert(reflect_struct::label<0>(vec) == "x");
    static_assert(reflect_struct::label<1>(vec) == "y");
    static_assert(reflect_struct::label<2>(vec) == "z");

    static_assert(reflect_struct::value<0>(vec) == 1.);
    static_assert(reflect_struct::value<1>(vec) == 2.);
    static_assert(reflect_struct::value<2>(vec) == 3.);

    static_assert(reflect_struct::entry<0>(vec) == std::pair{"x", 1.});
    static_assert(reflect_struct::entry<1>(vec) == std::pair{"y", 2.});
    static_assert(reflect_struct::entry<2>(vec) == std::pair{"z", 3.});
}

TEST_CASE("Tuple API") {
    constexpr vector vec = {1., 2., 3.};

    static_assert(std::get<0>(reflect_struct::label_view(vec)) == "x");
    static_assert(std::get<1>(reflect_struct::label_view(vec)) == "y");
    static_assert(std::get<2>(reflect_struct::label_view(vec)) == "z");

    static_assert(std::get<0>(reflect_struct::value_view(vec)) == 1.);
    static_assert(std::get<1>(reflect_struct::value_view(vec)) == 2.);
    static_assert(std::get<2>(reflect_struct::value_view(vec)) == 3.);

    static_assert(std::get<0>(reflect_struct::entry_view(vec)) == std::pair{"x", 1.});
    static_assert(std::get<1>(reflect_struct::entry_view(vec)) == std::pair{"y", 2.});
    static_assert(std::get<2>(reflect_struct::entry_view(vec)) == std::pair{"z", 3.});
}

// Looping over struct members
constexpr vector operator+(vector lhs, vector rhs) noexcept {
    vector res;

    reflect_struct::for_indices<vector>(
        [&](auto i) { reflect_struct::value<i>(res) = reflect_struct::value<i>(lhs) + reflect_struct::value<i>(rhs); });

    return res;
}

constexpr bool operator==(vector lhs, vector rhs) noexcept {
    bool res = true;

    reflect_struct::for_indices<vector>([&](auto i) {
        if (reflect_struct::value<i>(lhs) != reflect_struct::value<i>(rhs)) res = false;
    });

    return res;
}

static_assert(vector{1, 2, 3} + vector{4, 5, 6} == vector{5, 7, 9});