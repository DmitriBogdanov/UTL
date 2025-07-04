#include "include/UTL/integral.hpp"

using namespace utl;

// static_assert( std::size_t(15) < int(-7) == true );
// evaluates to 'true' due to implicit conversion, mathematically incorrect result,
// sensible compilers will issue a warning

static_assert( integral::cmp_less(std::size_t(15), int(-7)) == false );
// evaluates to 'false', mathematically correct result

int main() {}