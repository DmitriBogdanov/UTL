#include "include/UTL/math.hpp"

using namespace utl;

static_assert( math::kronecker_delta(3, 4) == 0 );
static_assert( math::kronecker_delta(3, 3) == 1 );

static_assert( math::levi_civita(3, 4, 4) ==  0 );
static_assert( math::levi_civita(3, 4, 5) ==  1 );
static_assert( math::levi_civita(5, 4, 3) == -1 );

int main() {}