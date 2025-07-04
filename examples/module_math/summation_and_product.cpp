#include "include/UTL/math.hpp"

using namespace utl;

constexpr auto  sum = math::sum( 1, 3, [](int i){ return math::sqr(i); }); // 1^2 + 2^2 + 3^2
constexpr auto prod = math::prod(1, 3, [](int i){ return math::sqr(i); }); // 1^2 * 2^2 * 3^2

static_assert(  sum == 1 + 4 + 9 );
static_assert( prod == 1 * 4 * 9 );

int main() {}