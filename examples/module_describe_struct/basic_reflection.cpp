#include "include/UTL/describe_struct.hpp"

// Define struct & reflection
struct quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_DESCRIBE_STRUCT(quaternion, r, i, j, k);

// Test basic reflection
using namespace utl;

static_assert( describe_struct::type_name<quaternion> == "quaternion" );

static_assert( describe_struct::size<quaternion> == 4 );

static_assert( describe_struct::names<quaternion>[0] == "r" );
static_assert( describe_struct::names<quaternion>[1] == "i" );
static_assert( describe_struct::names<quaternion>[2] == "j" );
static_assert( describe_struct::names<quaternion>[3] == "k" );

constexpr quaternion q = { 5., 6., 7., 8. };

static_assert( describe_struct::get<0>(q) == 5. );
static_assert( describe_struct::get<1>(q) == 6. );
static_assert( describe_struct::get<2>(q) == 7. );
static_assert( describe_struct::get<3>(q) == 8. );

int main() {}