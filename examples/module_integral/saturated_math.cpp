#include "include/UTL/integral.hpp"

using namespace utl;
using namespace integral::literals;

// Helper functions to avoid ugly casting
template <class T> constexpr T add(T lhs, T rhs) noexcept { return lhs + rhs; }
template <class T> constexpr T sub(T lhs, T rhs) noexcept { return lhs - rhs; }

// std::uint8_t has range [0, 255]
static_assert(               add<std::uint8_t>(255, 15) ==  14 ); // overflow
static_assert( integral::add_sat<std::uint8_t>(255, 15) == 255 ); // result gets clamped to max

// std::int8_t has range [-128, 127]
static_assert(               sub<std::int8_t>(-128, 14) ==  114 ); // underflow
// if we used 'int' instead of 'std::int8_t' this could even be UB due to underflow during signed
// arithmetic operation, for smaller types it's underflow during cast which is defined to wrap
static_assert( integral::sub_sat<std::int8_t>(-128, 14) == -128 ); // result gets clamped to min

// Saturated cast
static_assert( integral::saturate_cast<std::uint8_t>(  17) ==  17 ); // regular cast
static_assert( integral::saturate_cast<std::uint8_t>(1753) == 255 ); // value clamped to max
static_assert( integral::saturate_cast<std::uint8_t>(-143) ==   0 ); // value clamped to min

int main() {}