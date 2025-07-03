#include "include/UTL/enum_reflect.hpp"

// Register enum & reflection
enum class Side { LEFT = -1, RIGHT = 1, NONE = 0 };

UTL_ENUM_REFLECT(Side, LEFT, RIGHT, NONE);

// Test reflection
using namespace utl;
using namespace std::string_view_literals;

static_assert( enum_reflect::type_name<Side> == "Side" );

static_assert( enum_reflect::size<Side> == 3 );

static_assert( enum_reflect::names<Side>[0] == "LEFT"  );
static_assert( enum_reflect::names<Side>[1] == "RIGHT" );
static_assert( enum_reflect::names<Side>[2] == "NONE"  );

static_assert( enum_reflect::values<Side>[0] == Side::LEFT  );
static_assert( enum_reflect::values<Side>[1] == Side::RIGHT );
static_assert( enum_reflect::values<Side>[2] == Side::NONE  );

static_assert( enum_reflect::entries<Side>[0]  == std::pair{  "LEFT"sv, Side::LEFT  } );
static_assert( enum_reflect::entries<Side>[1]  == std::pair{ "RIGHT"sv, Side::RIGHT } );
static_assert( enum_reflect::entries<Side>[2]  == std::pair{  "NONE"sv, Side::NONE  } );

static_assert( enum_reflect::is_valid(Side{-1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 0}) == true  );
static_assert( enum_reflect::is_valid(Side{ 2}) == false );

static_assert( enum_reflect::to_underlying(Side::LEFT ) == -1 );
static_assert( enum_reflect::to_underlying(Side::RIGHT) ==  1 );
static_assert( enum_reflect::to_underlying(Side::NONE ) ==  0 );

static_assert( enum_reflect::to_string(Side::LEFT ) == "LEFT"  );
static_assert( enum_reflect::to_string(Side::RIGHT) == "RIGHT" );
static_assert( enum_reflect::to_string(Side::NONE ) == "NONE"  );

static_assert( enum_reflect::from_string<Side>("LEFT" ) == Side::LEFT  );
static_assert( enum_reflect::from_string<Side>("RIGHT") == Side::RIGHT );
static_assert( enum_reflect::from_string<Side>("NONE" ) == Side::NONE  );

int main() {}