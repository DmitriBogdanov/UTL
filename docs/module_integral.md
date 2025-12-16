[<img src ="images/badge_language_cpp_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_docs.svg">](https://dmitribogdanov.github.io/UTL/)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_workflow_windows.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_workflow_ubuntu.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_workflow_macos.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)
[<img src ="images/badge_workflow_freebsd.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/freebsd.yml)

# utl::integral

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/integral.hpp)

**utl::integral** module implements various utilities for dealing with integer types. Main features are:

- Integer division with different rounding modes
- Overflow/underflow detection
- Saturated math
- Heterogeneous (mathematically correct) integer comparison
- "Safe" integer casts
- Fixed-size & `std::size_t` literals

Such functionality is often useful in `constexpr` context when dealing with optimizations such as compile-time creation of bit-masks, bit-buffers, computation of different alignments, coefficients and etc. Also convenience.

> [!Note]
> Significant part of this module gets added into the standard library with **C++20** `<bits>` and `<utility>`, saturated math ships with **C++26** `<numeric>`. All such functions provide the same API as their `std::` variants to allow seamless future transition.

## Definitions

```cpp
// Rounding integer division
template <class T> constexpr T div_floor(T dividend, T divisor) noexcept;
template <class T> constexpr T div_ceil (T dividend, T divisor) noexcept;
template <class T> constexpr T div_down (T dividend, T divisor) noexcept;
template <class T> constexpr T div_up   (T dividend, T divisor) noexcept;

// Saturated math
template <class T> constexpr bool add_overflows(T lhs, T rhs) noexcept;
template <class T> constexpr bool sub_overflows(T lhs, T rhs) noexcept;
template <class T> constexpr bool mul_overflows(T lhs, T rhs) noexcept;
template <class T> constexpr bool div_overflows(T lhs, T rhs) noexcept;

template <class T> constexpr T add_sat(T lhs, T rhs) noexcept;
template <class T> constexpr T sub_sat(T lhs, T rhs) noexcept;
template <class T> constexpr T mul_sat(T lhs, T rhs) noexcept;
template <class T> constexpr T div_sat(T lhs, T rhs) noexcept;

// Heterogeneous integer comparison
template <class T1, class T2> constexpr bool cmp_equal        (T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_not_equal    (T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less         (T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater      (T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less_equal   (T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept;

template <class To, class From> constexpr bool in_range(From value) noexcept;

// Casts
template <class To, class From> constexpr To narrow_cast  (From value);
template <class To, class From> constexpr To saturate_cast(From value) noexcept;

template <class T> constexpr auto to_signed  (T value);
template <class T> constexpr auto to_unsigned(T value);

// Integer literals
namespace literals {
    constexpr std::int8_t    operator""_i8  (unsigned long long v) noexcept;
    constexpr std::uint8_t   operator""_u8  (unsigned long long v) noexcept;
    constexpr std::int16_t   operator""_i16 (unsigned long long v) noexcept;
    constexpr std::uint16_t  operator""_u16 (unsigned long long v) noexcept;
    constexpr std::int32_t   operator""_i32 (unsigned long long v) noexcept;
    constexpr std::uint32_t  operator""_u32 (unsigned long long v) noexcept;
    constexpr std::int64_t   operator""_i64 (unsigned long long v) noexcept;
    constexpr std::uint64_t  operator""_u64 (unsigned long long v) noexcept;
    constexpr std::size_t    operator""_sz  (unsigned long long v) noexcept;
    constexpr std::ptrdiff_t operator""_ptrd(unsigned long long v) noexcept;
}
```

## Methods

### Rounding integer division

> ```cpp
> template <class T> constexpr T div_floor(T dividend, T divisor) noexcept;
> template <class T> constexpr T div_ceil (T dividend, T divisor) noexcept;
> template <class T> constexpr T div_down (T dividend, T divisor) noexcept;
> template <class T> constexpr T div_up   (T dividend, T divisor) noexcept;
> ```

Returns the result of integer division with a given rounding mode.

| Function      | Rounding mode         |
| ------------- | --------------------- |
| `div_floor()` | Towards larger value  |
| `div_ceil()`  | Towards smaller value |
| `div_down()`  | Towards `0`           |
| `div_up()`    | Away from `0`         |

**Note:** There is a lot of partial or even blatantly erroneous implementations for this published online, the task is surprisingly tricky. Here signed values are properly handled and overflow behaves as it should.

### Saturated math

> ```cpp
> template <class T> constexpr bool add_overflows(T lhs, T rhs) noexcept;
> template <class T> constexpr bool sub_overflows(T lhs, T rhs) noexcept;
> template <class T> constexpr bool mul_overflows(T lhs, T rhs) noexcept;
> template <class T> constexpr bool div_overflows(T lhs, T rhs) noexcept;
> ```

Returns whether operator  `+`/`-`/`*`/`/` would overflow/underflow when applied to `lhs`, `rhs`.

> ```cpp
> template <class T> constexpr T add_sat(T lhs, T rhs) noexcept;
> template <class T> constexpr T sub_sat(T lhs, T rhs) noexcept;
> template <class T> constexpr T mul_sat(T lhs, T rhs) noexcept;
> template <class T> constexpr T div_sat(T lhs, T rhs) noexcept;
> ```

Returns the result of `+`/`-`/`*`/`/` computed in [saturated arithmetic](https://en.wikipedia.org/wiki/Saturation_arithmetic), which means instead of overflowing operations get clamped to a min/max value.

**Note:** This gets standardized in **C++26** as a part of [`<numeric>`](https://en.cppreference.com/w/cpp/header/numeric) header.

### Heterogeneous integer comparison

> ```cpp
> template <class T1, class T2> constexpr bool cmp_equal        (T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_not_equal    (T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_less         (T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_greater      (T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_less_equal   (T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept;
> ```

Functions that compare the values of two integers `lhs` and `rhs`. Unlike regular comparison operators, comparison is always mathematically correct for arbitrary types of `lhs` and `rhs`.

For example, `-1 > 0u` is `true` due to non-value-preserving integer conversion, while `cmp_greater(-1, 0u)` is `false` (as it should be mathematically).

**Note:** This gets standardized in **C++26** as [intcmp](https://en.cppreference.com/w/cpp/utility/intcmp).

> ```cpp
> template <class To, class From> constexpr bool in_range(From value) noexcept;
> ```

Returns whether `value` is in `[std::numeric_limits<To>::min(), std::numeric_limits<To>::max()]` range using heterogeneous comparison.

### Casts

> ```cpp
> template <class To, class From> constexpr To narrow_cast(From value);
> ```

Integer-to-integer cast that throws `std::domain_error` if conversion would overflow/underflow the result.

> ```cpp
> template <class To, class From> constexpr To saturate_cast(From value) noexcept;
> ```

Integer-to-integer cast that uses saturated math. If `value` lies outside of `[std::numeric_limits<To>::min(), std::numeric_limits<To>::max()]` range, it gets clamped to the appropriate side of that range.

> ```cpp
> template <class T> constexpr auto to_signed  (T value);
> template <class T> constexpr auto to_unsigned(T value);
> ```

Cast integer to a corresponding signed/unsigned type using `narrow_cast()`.

### Integer literals

> ```cpp
> namespace literals {
>     constexpr std::int8_t    operator""_i8  (unsigned long long v) noexcept;
>     constexpr std::uint8_t   operator""_u8  (unsigned long long v) noexcept;
>     constexpr std::int16_t   operator""_i16 (unsigned long long v) noexcept;
>     constexpr std::uint16_t  operator""_u16 (unsigned long long v) noexcept;
>     constexpr std::int32_t   operator""_i32 (unsigned long long v) noexcept;
>     constexpr std::uint32_t  operator""_u32 (unsigned long long v) noexcept;
>     constexpr std::int64_t   operator""_i64 (unsigned long long v) noexcept;
>     constexpr std::uint64_t  operator""_u64 (unsigned long long v) noexcept;
>     constexpr std::size_t    operator""_sz  (unsigned long long v) noexcept;
>     constexpr std::ptrdiff_t operator""_ptrd(unsigned long long v) noexcept;
> }
> ```

Literal suffixes for several integer types not included in `std`.

**Note 1:** Literals always evaluate to a valid value, if `v` doesn't convert to a valid value internal cast throws `std::domain_error` at `constexpr`, making it a compilation error.

**Note 2:** Literal for `std::size_t` gets standardized in **C++23** as a `zu` suffix. 

## Examples

### Integer division

[ [Run this code](https://godbolt.org/z/8rfGE1dKh) ] [ [Open source file](../examples/module_integral/integer_division.cpp) ]

```cpp
using namespace utl;

static_assert( integral::div_floor( 7, 5) == 1 ); // round to smaller
static_assert( integral::div_ceil ( 7, 5) == 2 ); // round to larger
static_assert( integral::div_down ( 7, 5) == 1 ); // round to 0
static_assert( integral::div_up   ( 7, 5) == 2 ); // round away from 0

static_assert( integral::div_floor(-7, 5) == -2 ); // round to smaller
static_assert( integral::div_ceil (-7, 5) == -1 ); // round to larger
static_assert( integral::div_down (-7, 5) == -1 ); // round to 0
static_assert( integral::div_up   (-7, 5) == -2 ); // round away from 0
```

### Saturated math

[ [Run this code](https://godbolt.org/z/6YerhvTGe) ] [ [Open source file](../examples/module_integral/saturated_math.cpp) ]

```cpp
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
```

### Heterogeneous comparison

[ [Run this code](https://godbolt.org/z/5W6EGW3dz) ] [ [Open source file](../examples/module_integral/heterogeneous_comparison.cpp) ]

```cpp
using namespace utl;

// static_assert( std::size_t(15) < int(-7) == true );
// evaluates to 'true' due to implicit conversion, mathematically incorrect result,
// sensible compilers will issue a warning

static_assert( integral::cmp_less(std::size_t(15), int(-7)) == false );
// evaluates to 'false', mathematically correct result
```

### Narrow cast

[ [Run this code](https://godbolt.org/z/sch65GG7e) ] [ [Open source file](../examples/module_integral/narrow_cast.cpp) ]

```cpp
try {
    using namespace utl;

    // Narrow cast
    [[maybe_unused]] char c1 =           static_cast<char>(  34); // this is fine, returns 34
    [[maybe_unused]] char c2 = integral::narrow_cast<char>(  34); // this is fine, returns 34
    [[maybe_unused]] char c3 =           static_cast<char>(1753); // silently overflows, returns -39
    [[maybe_unused]] char c4 = integral::narrow_cast<char>(1753); // throws 'std::domain_error'

} catch (std::domain_error &e) { 
    std::cerr << "ERROR: Caught exception:\n\n" << e.what();
}
```

Output:

```
ERROR: Caught exception:

narrow_cast() overflows the result.
```

### Sign conversion

[ [Run this code](https://godbolt.org/z/YhbYoMWhj) ] [ [Open source file](../examples/module_integral/sign_conversion.cpp) ]

```cpp
try {
    constexpr int N = -14;

    // for (std::size_t i = 0; i < N; ++i) std::cout << i;
    // compiler warns about signed/unsigned comparison, doesn't compile with -Werror

    // for (std::size_t i = 0; i < static_cast<std::size_t>(N); ++i) std::cout << i;
    // casts 'N' to '18446744073709551602' since we forgot to check for negative 'N'

    for (std::size_t i = 0; i < utl::integral::to_unsigned(N); ++i) std::cout << i;
    // this is good, comparison is unsigned/unsigned and incorrect 'N' will cause an exception

} catch (std::domain_error &e) {
    std::cerr << "ERROR: Caught exception:\n\n" << e.what();
}
```

Output:

```
ERROR: Caught exception:

narrow_cast() overflows the result.
```

### Integral literals

[ [Run this code](https://godbolt.org/z/W687695c9) ] [ [Open source file](../examples/module_integral/integral_literals.cpp) ]

```cpp
using namespace utl::integral::literals;

// constexpr auto x = 129_i8;
// won't compile, std::int8_t has range [-128, 127]

constexpr auto x = 124_i8;
// this is fine, 'x' has type 'std::int8_t'

// constexpr auto x = -17_i8;
// be wary of this, C++ has no concept of signed literals and treats it as an unary minus
// applied to 'std::int8_t', which triggers integer promotion and returns an 'int'

static_assert(sizeof(x) == 1);
```