#include "include/UTL/describe_struct.hpp"

using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

constexpr vector vec = { 1., 2., 3. };

// Access it like a tuple
static_assert( std::get<0>(describe_struct::label_view(vec)) == "x" );
static_assert( std::get<1>(describe_struct::label_view(vec)) == "y" );
static_assert( std::get<2>(describe_struct::label_view(vec)) == "z" );

static_assert( std::get<0>(describe_struct::value_view(vec)) ==  1. );
static_assert( std::get<1>(describe_struct::value_view(vec)) ==  2. );
static_assert( std::get<2>(describe_struct::value_view(vec)) ==  3. );

static_assert( std::get<0>(describe_struct::entry_view(vec)).first  == "x" );
static_assert( std::get<1>(describe_struct::entry_view(vec)).first  == "y" );
static_assert( std::get<2>(describe_struct::entry_view(vec)).first  == "z" );

static_assert( std::get<0>(describe_struct::entry_view(vec)).second ==  1. );
static_assert( std::get<1>(describe_struct::entry_view(vec)).second ==  2. );
static_assert( std::get<2>(describe_struct::entry_view(vec)).second ==  3. );

int main() {}