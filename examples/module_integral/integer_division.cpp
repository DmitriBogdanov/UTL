#include "include/UTL/integral.hpp"

using namespace utl;

static_assert( integral::div_floor( 7, 5) == 1 ); // round to smaller
static_assert( integral::div_ceil ( 7, 5) == 2 ); // round to larger
static_assert( integral::div_down ( 7, 5) == 1 ); // round to 0
static_assert( integral::div_up   ( 7, 5) == 2 ); // round away from 0

static_assert( integral::div_floor(-7, 5) == -2 ); // round to smaller
static_assert( integral::div_ceil (-7, 5) == -1 ); // round to larger
static_assert( integral::div_down (-7, 5) == -1 ); // round to 0
static_assert( integral::div_up   (-7, 5) == -2 ); // round away from 0

int main() {}