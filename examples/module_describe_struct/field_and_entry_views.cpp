#include "include/UTL/describe_struct.hpp"

// Define struct & reflection
struct quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_DESCRIBE_STRUCT(quaternion, r, i, j, k);

// Test field & entry views
using namespace utl;

constexpr quaternion q = { 5., 6., 7., 8. };

static_assert( describe_struct::field_view(q) == std::tuple{ 5., 6., 7., 8. } );

static_assert( std::get<0>(describe_struct::entry_view(q)).first  == "r" );
static_assert( std::get<0>(describe_struct::entry_view(q)).second == 5.  );
static_assert( std::get<1>(describe_struct::entry_view(q)).first  == "i" );
static_assert( std::get<1>(describe_struct::entry_view(q)).second == 6.  );
static_assert( std::get<2>(describe_struct::entry_view(q)).first  == "j" );
static_assert( std::get<2>(describe_struct::entry_view(q)).second == 7.  );
static_assert( std::get<3>(describe_struct::entry_view(q)).first  == "k" );
static_assert( std::get<3>(describe_struct::entry_view(q)).second == 8.  );

int main() {}