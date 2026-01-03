#include "include/UTL/reflect_struct.hpp"

#include <concepts>

// Resolve defect report LWG 3865 ('std::pair' should accept heterogenous comparison)
// unless the compiler already implements a fix (which we can detect with a concept)
template <class T1, class T2, class U1, class U2>
    requires ( !requires { { std::pair<T1, T2>{} != std::pair<U1, U2>{} } -> std::same_as<bool>; } )
constexpr bool operator==(const std::pair<T1, T2>& lhs, const std::pair<U1, U2>& rhs) {
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

using namespace utl;

// Define structure
struct vector { double x, y, z; };

// General reflection
static_assert( reflect_struct::name<vector> == "vector" );
static_assert( reflect_struct::size<vector> == 3        );

// Member reflection
constexpr vector vec = { 1., 2., 3. };

static_assert( reflect_struct::label<0>(vec) == "x" );
static_assert( reflect_struct::label<1>(vec) == "y" );
static_assert( reflect_struct::label<2>(vec) == "z" );

static_assert( reflect_struct::value<0>(vec) ==  1. );
static_assert( reflect_struct::value<1>(vec) ==  2. );
static_assert( reflect_struct::value<2>(vec) ==  3. );

static_assert( reflect_struct::entry<0>(vec) == std::pair{ "x", 1. } );
static_assert( reflect_struct::entry<1>(vec) == std::pair{ "y", 2. } );
static_assert( reflect_struct::entry<2>(vec) == std::pair{ "z", 3. } );

int main() {}