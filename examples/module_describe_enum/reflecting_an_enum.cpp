#include "include/UTL/describe_enum.hpp"

// Register enum & reflection
enum class side { left = -1, right = 1, none = 0 };

UTL_DESCRIBE_ENUM(side, left, right, none);

// Test reflection
using namespace utl;
using namespace std::string_view_literals;

static_assert( describe_enum::name<side> == "side" );

static_assert( describe_enum::size<side> == 3 );

static_assert( describe_enum::label_array<side>[0] == "left"  );
static_assert( describe_enum::label_array<side>[1] == "right" );
static_assert( describe_enum::label_array<side>[2] == "none"  );

static_assert( describe_enum::value_array<side>[0] == side::left  );
static_assert( describe_enum::value_array<side>[1] == side::right );
static_assert( describe_enum::value_array<side>[2] == side::none  );

static_assert( describe_enum::entry_array<side>[0]  == std::pair{  "left"sv, side::left  } );
static_assert( describe_enum::entry_array<side>[1]  == std::pair{ "right"sv, side::right } );
static_assert( describe_enum::entry_array<side>[2]  == std::pair{  "none"sv, side::none  } );

static_assert( describe_enum::is_valid(side{-1}) == true  );
static_assert( describe_enum::is_valid(side{ 1}) == true  );
static_assert( describe_enum::is_valid(side{ 0}) == true  );
static_assert( describe_enum::is_valid(side{ 2}) == false );

static_assert( describe_enum::to_underlying(side::left ) == -1 );
static_assert( describe_enum::to_underlying(side::right) ==  1 );
static_assert( describe_enum::to_underlying(side::none ) ==  0 );

static_assert( describe_enum::to_string(side::left ) == "left"  );
static_assert( describe_enum::to_string(side::right) == "right" );
static_assert( describe_enum::to_string(side::none ) == "none"  );

static_assert( describe_enum::from_string<side>("left" ) == side::left  );
static_assert( describe_enum::from_string<side>("right") == side::right );
static_assert( describe_enum::from_string<side>("none" ) == side::none  );

int main() {}