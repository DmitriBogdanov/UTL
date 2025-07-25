[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::math

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/math.hpp)

**utl::math** header is a collection of mathematical utils that includes:

- Constants
- Template math
- Non-overflowing functions
- Index functions
- Conversions and etc.

All implementations `template`, `constexpr` and `noexcept`.

> [!Note]
> For a more "complete" mathematical library check out [DmitriBogdanov/GSE](https://github.com/DmitriBogdanov/GSE).

## Definitions

```cpp
// Constants
namespace constants {
    constexpr double pi      = 3.14159265358979323846;
    constexpr double two_pi  = 2.0 * pi;
    constexpr double half_pi = 0.5 * pi;
    constexpr double inv_pi  = 1.0 / pi;
    constexpr double e       = 2.71828182845904523536;
    constexpr double phi     = 1.61803398874989484820;
}

// Basic functions
template <class T> constexpr T       abs(T x) noexcept;
template <class T> constexpr T      sign(T x) noexcept;
template <class T> constexpr T     bsign(T x) noexcept;
template <class T> constexpr T       sqr(T x) noexcept;
template <class T> constexpr T      cube(T x) noexcept;
template <class T> constexpr T       inv(T x) noexcept;
template <class T> constexpr T heaviside(T x) noexcept;

// Non-overflowing functions
template <class T> constexpr T midpoint(T a, T b) noexcept;
template <class T> constexpr T  absdiff(T a, T b) noexcept;

// Power functions
template <class T, class U> constexpr T     pow(T x, U p) noexcept;
template <         class U> constexpr U signpow(     U p) noexcept;

// Index functions
template <class T> constexpr T kronecker_delta(T i, T j     ) noexcept;
template <class T> constexpr T     levi_civita(T i, T j, T k) noexcept;

// Conversions
template <class T> constexpr T deg_to_rad(T degrees) noexcept;
template <class T> constexpr T rad_to_deg(T radians) noexcept;

// Sequence operations
template <class Idx, class Func> constexpr auto  sum(Idx low, Idx high, Func&& func);
template <class Idx, class Func> constexpr auto prod(Idx low, Idx high, Func&& func);
```

> [!Note]
> All methods have appropriate SFINAE-restrictions, which are omitted in documentation to reduce verbosity.

## Methods

### Constants

> ```cpp
> namespace constants {
>     constexpr double pi      = 3.14159265358979323846;
>     constexpr double two_pi  = 2.0 * pi;
>     constexpr double half_pi = 0.5 * pi;
>     constexpr double inv_pi  = 1.0 / pi;
>     constexpr double e       = 2.71828182845904523536;
>     constexpr double phi     = 1.61803398874989484820;
> }
> ```

Basic mathematical constants. In **C++20** most of these get standardized as a part of [`<numbers>`](https://en.cppreference.com/w/cpp/numeric/constants) header.

### Basic functions

> ```cpp
> template <class T> constexpr T       abs(T x) noexcept;
> template <class T> constexpr T      sign(T x) noexcept;
> template <class T> constexpr T     bsign(T x) noexcept;
> template <class T> constexpr T       sqr(T x) noexcept;
> template <class T> constexpr T      cube(T x) noexcept;
> template <class T> constexpr T       inv(T x) noexcept;
> template <class T> constexpr T heaviside(T x) noexcept;
> ```

Returns $|x|$, $\mathrm{sign} (x)$, $x^2$, $x^3$, $x^{-1}$, or $H(x)$. 

`T` can be of any arithmetic type except `bool`.

To reduce integer division surprises, `inv()` also requires a floating point.

**Note 1:** `sign()` is a standard sign function which returns $-1$, $0$ or $1$. `bsign()` is a binary variation that returns $1$ instead of $0$.

**Note 2:** $H(x)$ refers to [Heaviside step function](https://en.wikipedia.org/wiki/Heaviside_step_function).

### Non-overflowing functions

> ```cpp
> template <class T> constexpr T midpoint(T a, T b) noexcept;
> template <class T> constexpr T  absdiff(T a, T b) noexcept;
> ```

Computes $\dfrac{a + b}{2}$ or $|a - b|$ without potential overflow.

`T` can be of any arithmetic type except `bool`.

Integer `midpoint()` rounds down in case of a fractional result.

**Note 1:** Correctly computing integer / float `midpoint()` is trickier than it might initially seem, naive `(a + b) / 2` formula suffers multiple issues when applied to values near min / max of the given type.

**Note 2:** Compared to integer `std::midpoint()` (which rounds towards `a`), integer `math::midpoint()` (which rounds down) is commutative and faster to compute. Floating point behavior is the same.

### Power functions

> ```cpp
> template <class T, class U> constexpr T     pow(T x, U p) noexcept;
> template <         class U> constexpr U signpow(     U p) noexcept;
> ```

Returns $x^p$ or $-1^p$ .

`T` can be of any arithmetic type except `bool`.

`U` can be of any integral type.

**Note:** Computing `signpow(p)` is significantly faster than `std::pow(-1, p)`.

### Index functions

> ```cpp
> template <class T> constexpr T kronecker_delta(T i, T j) noexcept;
> ```

Computes [Kronecker delta](https://en.wikipedia.org/wiki/Kronecker_delta) symbol: `1` if `i == j`, `0` otherwise.

`T` can be of any integral type.

> ```cpp
> template <class T> constexpr T levi_civita(T i, T j, T k) noexcept;
> ```

Computes [Levi-Civita](https://en.wikipedia.org/wiki/Levi-Civita_symbol) symbol: `1` if `(i, j, k)` form an [even permutation](https://en.wikipedia.org/wiki/Parity_of_a_permutation), `-1` if `(i, j, k)` form an odd permutation, and `0` if any the adjacent letters are equal.

`T` can be of any integral type.

### Conversions

> ```cpp
> template <class T> constexpr T deg_to_rad(T degrees) noexcept;
> template <class T> constexpr T rad_to_deg(T radians) noexcept;
> ```

Converts degrees to radians and back.

`T` can be of any floating point type.

### Sequence operations

> ```cpp
> template <class Idx, class Func> constexpr auto  sum(Idx low, Idx high, Func&& f);
> template <class Idx, class Func> constexpr auto prod(Idx low, Idx high, Func&& f);
> ```

Computes $\sum_{i = low}^{high} f(i)$ or $\prod_{i = low}^{high} f(i)$.

`Idx` can be of any integral type.

`noexcept` when `f` is non-throwing.

## Examples

### Template math functions

[ [Run this code](https://godbolt.org/z/P9xbPPsnq) ]

```cpp
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
```

### Index functions

[ [Run this code](https://godbolt.org/z/aqEMqhrdd) ]

```cpp
using namespace utl;

static_assert( math::kronecker_delta(3, 4) == 0 );
static_assert( math::kronecker_delta(3, 3) == 1 );

static_assert( math::levi_civita(3, 4, 4) ==  0 );
static_assert( math::levi_civita(3, 4, 5) ==  1 );
static_assert( math::levi_civita(5, 4, 3) == -1 );
```

### Conversions

[ [Run this code](https://godbolt.org/z/zd59Pox6K) ]

```cpp
using namespace utl;

static_assert( math::absdiff(math::deg_to_rad(180.), math::constants::pi) < 1e-16  );

static_assert( math::absdiff(math::rad_to_deg(math::constants::pi), 180.) < 1e-16  );
```

### Summation & product

[ [Run this code](https://godbolt.org/z/Ejaj6z4vs) ]

```cpp
using namespace utl;

constexpr auto  sum = math::sum( 1, 3, [](int i){ return math::sqr(i); }); // 1^2 + 2^2 + 3^2
constexpr auto prod = math::prod(1, 3, [](int i){ return math::sqr(i); }); // 1^2 * 2^2 * 3^2

static_assert(  sum == 1 + 4 + 9 );
static_assert( prod == 1 * 4 * 9 );
```

## Newer standards

Some parts of this header can be replaced with `std` features in newer standards:

- **C++20** adds mathematical constants as [`<numbers>`](https://en.cppreference.com/w/cpp/numeric/constants.html)
- **C++20** adds [`std::midpoint()`](https://en.cppreference.com/w/cpp/numeric/midpoint.html)
- **C++23** adds `constexpr` support to [`<cmath>`](https://en.cppreference.com/w/cpp/header/cmath) functions