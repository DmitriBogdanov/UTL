#include "include/UTL/struct_reflect.hpp"

// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test basic reflection
using namespace utl;

static_assert( struct_reflect::type_name<Quaternion> == "Quaternion" );

static_assert( struct_reflect::size<Quaternion> == 4 );

static_assert( struct_reflect::names<Quaternion>[0] == "r" );
static_assert( struct_reflect::names<Quaternion>[1] == "i" );
static_assert( struct_reflect::names<Quaternion>[2] == "j" );
static_assert( struct_reflect::names<Quaternion>[3] == "k" );

constexpr Quaternion q = { 5., 6., 7., 8. };

static_assert( struct_reflect::get<0>(q) == 5. );
static_assert( struct_reflect::get<1>(q) == 6. );
static_assert( struct_reflect::get<2>(q) == 7. );
static_assert( struct_reflect::get<3>(q) == 8. );

int main() {}