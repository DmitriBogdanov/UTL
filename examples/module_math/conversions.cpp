#include "include/UTL/math.hpp"

using namespace utl;

static_assert( math::absdiff(math::deg_to_rad(180.), math::constants::pi) < 1e-16  );

static_assert( math::absdiff(math::rad_to_deg(math::constants::pi), 180.) < 1e-16  );

int main() {}