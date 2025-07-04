#include "include/UTL/math.hpp"

using namespace utl;

static_assert( math::abs      (-7 ) ==  7  );
static_assert( math::sign     ( 0 ) ==  0  );
static_assert( math::bsign    ( 0 ) ==  1  );
static_assert( math::sqr      (-2 ) ==  4  );
static_assert( math::cube     (-2 ) == -8  );
static_assert( math::inv      ( 2.) == 0.5 );
static_assert( math::heaviside( 2.) ==  1  );

static_assert( math::midpoint(3, 5) == 4 );
static_assert( math::absdiff (4, 7) == 3 );

static_assert( math::pow    (2, 7) ==  128 );
static_assert( math::signpow(  -5) == -1   );

int main() {}