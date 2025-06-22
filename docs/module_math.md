[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

# utl::math

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/math.hpp)

**utl::math** header is a collection of various mathematical utils that includes:

- Constants
- Template math functions
- Indicator functions, index functions, conversions and etc.

This is useful for simplifying mathematical expressions and template math. All provided implementations are non-throwing `constexpr`.

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

template <class T> constexpr T midpoint(T a, T b) noexcept;
template <class T> constexpr T  absdiff(T a, T b) noexcept;

// Power functions
template <class T, class U> constexpr T     pow(T x, U p) noexcept;
template <         class U> constexpr U signpow(     U p) noexcept;

// Index functions
template <class Integer> constexpr T kronecker_delta(T i, T j     ) noexcept;
template <class Integer> constexpr T     levi_civita(T i, T j, T k) noexcept;

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

> ```cpp
> template <class T> constexpr T midpoint(T a, T b) noexcept;
> template <class T> constexpr T  absdiff(T a, T b) noexcept;
> ```

Returns $\dfrac{a + b}{2}$ or $|a - b|$ of an appropriate type.

`T` can be of any arithmetic type except `bool`.

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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCBmpAAOqAqETgwe3r56icmOAkEh4SxRMXF2mA6pQgRMxATpPn5ctpj2uQxVNQT5YZHRsbbVtfWZTQqDXcE9RX1mAJS2qF7EyOwc5gDMwcjeWADUJutuCAQE8QogAPQXxEwA7gB0wIQIXhFeSsuyjAT3aCwXABEWIRiHgLKhgOhDKgAG4XOQAFQCFxYTDG0QuWx2mHhSJRTAICHuCHi8QO2BMGgAgpSqcECLtUcEILN9gB2KzU3bc3bvYLAXbMNgKeJMFa8gi0A6cmlcnljAl4ZAAfTRHwIEEZBKQICYEQUPJ5EAAtGzdqyDgDLdyzebpbTDQrHCq1dENVrCSAQMlgAxDbtNRpzft1lbQ9yg3b1jLHdVnaqFOrNajPSB9XhfYbA8HLdbdlxubN7XLuU6lQmkx6dQoAI7Ef0B41mHOhvOSQvFqmxxUuxNu5Par3IV6Yf0m5sW1vh40ADg70Yd8rj5ddtQHqeCMIbmrM90nYYBuw09wArFGY0uexX%2B1WvQhMEwYXhklgd3uQwfuQXz7TF6Xl72lYpjqwLoIk9IQOspC7Ce%2B7Wu2RYLiWuxloBN7AV6eoKPgVBUAG0i7GycHhusP7Un%2BKEAdea63iAiS3FmcSEcRh75mYc6IRe/5Xqu7oYd6GYMPRmq7MasEftaxrfmRNJsla1IcPMtCcCevB%2BBwWikKgnBuNY1goYsyyjhsPCkAQmiKfMADWIBslw9x2WyM7rFIM5mAAbAAnFwGgnvonCSGpFlaZwvDnBoZkWfMcCwDAiAoKgLDxHQ0TkJQfzJfQMTbIYwBcO5GgRTQtAENE5wQBEwURMENQAJ6cKZ1XMMQtUAPIRNo5TmdwvB/GwgitQwtD1RpvBYG8wBuGItDnD1pBYKiRjiKN814MQXV4DCmCzZpmCqOUXilcF9ItMFtB4BENwtR4WDBQQoIsA1vBbcQERJJgAKYItwDnUYUV8AYwAKAAangmC3K18SME9MiCCIYjsFIsPyEoajBboTQGH9piWNY%2BgXeckDzKg8RtLNxpjOglo41YlhcGaxqtWYvCwtEoJYITLLNK0qQuAw7ieA0/j890hTFFkSQpAIwyNAkkttKLvQxKMLQbQIHRDILIzc2r7TjIr0zKwMnQy3oCq1Ab4tcPMCiGSsEhKSpQUrdpHC7KoM7uca7ntjlRj5u59zHkGEC4IQJD7GYLmzLw3VaLM8z3kwWAxFzykcIFpCPSeEXqZprthSAEVx4ppAxfFiynIdaUQBlKXEKErCrB7Xs%2B7sfsCvlQdB7wmD4EQ7N6PwcOiOISPDyjKjqCtGOkLcNzxE9jscKppB5yznCtYd8SHbsqB4S33u%2B1jneB8HAYeEl9eR9HsdRdZICSIHXmSGYXCe75nl2fl/kZ7w2e52CgXWwRdIqjQTqQGy7F7jsTfu5GcJ56brA0DOTyT9f7rGdvnUKYD47L2ZmvIBOCS4QJeskZwkggA%3D%3D) ]

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:2,positionColumn:1,positionLineNumber:2,selectionStartColumn:1,selectionStartLineNumber:2,startColumn:1,startLineNumber:2),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/math.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++static_assert(+math::kronecker_delta(3,+4)+%3D%3D+0+)%3B%0A++++static_assert(+math::kronecker_delta(3,+3)+%3D%3D+1+)%3B%0A%0A++++static_assert(+math::levi_civita(3,+4,+4)+%3D%3D++0+)%3B%0A++++static_assert(+math::levi_civita(3,+4,+5)+%3D%3D++1+)%3B%0A++++static_assert(+math::levi_civita(5,+4,+3)+%3D%3D+-1+)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

static_assert( math::kronecker_delta(3, 4) == 0 );
static_assert( math::kronecker_delta(3, 3) == 1 );

static_assert( math::levi_civita(3, 4, 4) ==  0 );
static_assert( math::levi_civita(3, 4, 5) ==  1 );
static_assert( math::levi_civita(5, 4, 3) == -1 );
```

### Conversions

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:5,positionColumn:1,positionLineNumber:5,selectionStartColumn:1,selectionStartLineNumber:5,startColumn:1,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/math.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++static_assert(+math::absdiff(math::deg_to_rad(180.),+math::constants::pi)+%3C+1e-16++)%3B%0A%0A++++static_assert(+math::absdiff(math::rad_to_deg(math::constants::pi),+180.)+%3C+1e-16++)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

static_assert( math::absdiff(math::deg_to_rad(180.), math::constants::pi) < 1e-16  );

static_assert( math::absdiff(math::rad_to_deg(math::constants::pi), 180.) < 1e-16  );
```

### Summation & product

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:25,endLineNumber:4,positionColumn:25,positionLineNumber:4,selectionStartColumn:25,selectionStartLineNumber:4,startColumn:25,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/math.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++constexpr+auto++sum+%3D+math::sum(+1,+3,+%5B%5D(int+i)%7B+return+math::sqr(i)%3B+%7D)%3B+//+1%5E2+%2B+2%5E2+%2B+3%5E2%0A++++constexpr+auto+prod+%3D+math::prod(1,+3,+%5B%5D(int+i)%7B+return+math::sqr(i)%3B+%7D)%3B+//+1%5E2+*+2%5E2+*+3%5E2%0A%0A++++static_assert(++sum+%3D%3D+1+%2B+4+%2B+9+)%3B%0A++++static_assert(+prod+%3D%3D+1+*+4+*+9+)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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