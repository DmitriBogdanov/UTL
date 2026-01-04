#include "include/UTL/describe_struct.hpp"

using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

// General reflection
static_assert( describe_struct::name<vector> == "vector" );
static_assert( describe_struct::size<vector> == 3        );

// Member reflection
constexpr vector vec = { 1., 2., 3. };

static_assert( describe_struct::label<0>(vec) == "x" );
static_assert( describe_struct::label<1>(vec) == "y" );
static_assert( describe_struct::label<2>(vec) == "z" );

static_assert( describe_struct::value<0>(vec) ==  1. );
static_assert( describe_struct::value<1>(vec) ==  2. );
static_assert( describe_struct::value<2>(vec) ==  3. );

static_assert( describe_struct::entry<0>(vec).first  == "x" );
static_assert( describe_struct::entry<1>(vec).first  == "y" );
static_assert( describe_struct::entry<2>(vec).first  == "z" );

static_assert( describe_struct::entry<0>(vec).second ==  1. );
static_assert( describe_struct::entry<1>(vec).second ==  2. );
static_assert( describe_struct::entry<2>(vec).second ==  3. );


int main() {}