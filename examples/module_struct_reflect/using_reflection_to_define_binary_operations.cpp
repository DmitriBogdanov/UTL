#include "include/UTL/struct_reflect.hpp"

// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Define binary operation (member-wise addition)
constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    Quaternion res = lhs;
    utl::struct_reflect::for_each(res, rhs, [&](auto& l, const auto& r){ l += r; });
    return res;
}

// Define binary operation with predicates (member-wise equality)
constexpr bool operator==(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    return utl::struct_reflect::true_for_all(lhs, rhs, [&](const auto& l, const auto& r){ return l == r; });
}

// Test operations
static_assert( Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} == Quaternion{6, 8, 10, 12} );

int main() {}