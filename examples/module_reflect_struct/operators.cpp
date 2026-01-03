#include "include/UTL/reflect_struct.hpp"

using namespace utl;

// Define structure
struct vector { double x, y, z; };

// Member-wise 'operator+'
constexpr vector operator+(vector lhs, vector rhs) noexcept {
    vector res;

    reflect_struct::for_indices<vector>([&] (auto i) {
        reflect_struct::value<i>(res) = reflect_struct::value<i>(lhs) + reflect_struct::value<i>(rhs);
    });

    return res;
}

// Member-wise 'operator=='
constexpr bool operator==(vector lhs, vector rhs) noexcept {
    bool res = true;

    reflect_struct::for_indices<vector>([&] (auto i) {
        if (reflect_struct::value<i>(lhs) != reflect_struct::value<i>(rhs)) res = false;
    });

    return res;
}

static_assert( vector{1, 2, 3} + vector{4, 5, 6} == vector{5, 7, 9} );

int main() {}