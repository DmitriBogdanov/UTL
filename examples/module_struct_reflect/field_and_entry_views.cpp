#include "include/UTL/struct_reflect.hpp"

// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test field & entry views
using namespace utl;

constexpr Quaternion q = { 5., 6., 7., 8. };

static_assert( struct_reflect::field_view(q) == std::tuple{ 5., 6., 7., 8. } );

static_assert( std::get<0>(struct_reflect::entry_view(q)).first  == "r" );
static_assert( std::get<0>(struct_reflect::entry_view(q)).second == 5.  );
static_assert( std::get<1>(struct_reflect::entry_view(q)).first  == "i" );
static_assert( std::get<1>(struct_reflect::entry_view(q)).second == 6.  );
static_assert( std::get<2>(struct_reflect::entry_view(q)).first  == "j" );
static_assert( std::get<2>(struct_reflect::entry_view(q)).second == 7.  );
static_assert( std::get<3>(struct_reflect::entry_view(q)).first  == "k" );
static_assert( std::get<3>(struct_reflect::entry_view(q)).second == 8.  );

int main() {}