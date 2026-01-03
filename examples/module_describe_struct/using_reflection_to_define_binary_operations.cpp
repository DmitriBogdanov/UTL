#include "include/UTL/describe_struct.hpp"

// Define struct & reflection
struct quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_DESCRIBE_STRUCT(quaternion, r, i, j, k);

// Define binary operation (member-wise addition)
constexpr quaternion operator+(const quaternion& lhs, const quaternion &rhs) noexcept {
    quaternion res = lhs;
    utl::describe_struct::for_each(res, rhs, [&](auto& l, const auto& r){ l += r; });
    return res;
}

// Define binary operation with predicates (member-wise equality)
constexpr bool operator==(const quaternion& lhs, const quaternion &rhs) noexcept {
    return utl::describe_struct::true_for_all(lhs, rhs, [&](const auto& l, const auto& r){ return l == r; });
}

// Test operations
static_assert( quaternion{1, 2, 3, 4} + quaternion{5, 6, 7, 8} == quaternion{6, 8, 10, 12} );

int main() {}