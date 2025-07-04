#include "include/UTL/integral.hpp"

using namespace utl::integral::literals;

// constexpr auto x = 129_i8;
// won't compile, std::int8_t has range [-128, 127]

constexpr auto x = 124_i8;
// this is fine, 'x' has type 'std::int8_t'

// constexpr auto x = -17_i8;
// be wary of this, C++ has no concept of signed literals and treats it as an unary minus
// applied to 'std::int8_t', which triggers integer promotion and returns an 'int'

static_assert(sizeof(x) == 1);

int main() {}