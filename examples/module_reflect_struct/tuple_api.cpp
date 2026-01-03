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

constexpr vector vec = { 1., 2., 3. };

// Access it like a tuple
static_assert( std::get<0>(reflect_struct::label_view(vec)) == "x" );
static_assert( std::get<1>(reflect_struct::label_view(vec)) == "y" );
static_assert( std::get<2>(reflect_struct::label_view(vec)) == "z" );

static_assert( std::get<0>(reflect_struct::value_view(vec)) ==  1. );
static_assert( std::get<1>(reflect_struct::value_view(vec)) ==  2. );
static_assert( std::get<2>(reflect_struct::value_view(vec)) ==  3. );

static_assert( std::get<0>(reflect_struct::entry_view(vec)) ==  std::pair{ "x", 1. } );
static_assert( std::get<1>(reflect_struct::entry_view(vec)) ==  std::pair{ "y", 2. } );
static_assert( std::get<2>(reflect_struct::entry_view(vec)) ==  std::pair{ "z", 3. } );

int main() {}