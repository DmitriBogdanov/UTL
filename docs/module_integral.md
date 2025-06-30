[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::integral

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/integral.hpp)

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
template <class T> constexpr T div_ceil( T dividend, T divisor) noexcept;
template <class T> constexpr T div_down( T dividend, T divisor) noexcept;
template <class T> constexpr T div_up(   T dividend, T divisor) noexcept;

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
template <class T1, class T2> constexpr bool cmp_equal(        T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_not_equal(    T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less(         T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater(      T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_less_equal(   T1 lhs, T2 rhs) noexcept;
template <class T1, class T2> constexpr bool cmp_greater_equal(T1 lhs, T2 rhs) noexcept;

template <class To, class From> constexpr bool in_range(From value) noexcept;

// Casts
template <class To, class From> constexpr To narrow_cast(  From value);
template <class To, class From> constexpr To saturate_cast(From value) noexcept;

template <class T> constexpr auto to_signed(  T value);
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
> template <class T> constexpr T div_ceil( T dividend, T divisor) noexcept;
> template <class T> constexpr T div_down( T dividend, T divisor) noexcept;
> template <class T> constexpr T div_up(   T dividend, T divisor) noexcept;
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
> template <class T1, class T2> constexpr bool cmp_equal(        T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_not_equal(    T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_less(         T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_greater(      T1 lhs, T2 rhs) noexcept;
> template <class T1, class T2> constexpr bool cmp_less_equal(   T1 lhs, T2 rhs) noexcept;
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
> template <class T> constexpr auto to_signed(  T value);
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:72,endLineNumber:13,positionColumn:72,positionLineNumber:13,selectionStartColumn:72,selectionStartLineNumber:13,startColumn:72,startLineNumber:13),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Ausing+namespace+utl%3B%0A%0Astatic_assert(+integral::div_floor(7,+5)+%3D%3D+1+)%3B+//+round+to+smaller%0Astatic_assert(+integral::div_ceil(+7,+5)+%3D%3D+2+)%3B+//+round+to+larger%0Astatic_assert(+integral::div_down(+7,+5)+%3D%3D+1+)%3B+//+round+to+0%0Astatic_assert(+integral::div_up(+++7,+5)+%3D%3D+2+)%3B+//+round+away+from+0%0A%0Astatic_assert(+integral::div_floor(-7,+5)+%3D%3D+-2+)%3B+//+round+to+smaller%0Astatic_assert(+integral::div_ceil(+-7,+5)+%3D%3D+-1+)%3B+//+round+to+larger%0Astatic_assert(+integral::div_down(+-7,+5)+%3D%3D+-1+)%3B+//+round+to+0%0Astatic_assert(+integral::div_up(+++-7,+5)+%3D%3D+-2+)%3B+//+round+away+from+0%0A%0Aint+main()+%7B%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

static_assert( integral::div_floor(7, 5) == 1 ); // round to smaller
static_assert( integral::div_ceil( 7, 5) == 2 ); // round to larger
static_assert( integral::div_down( 7, 5) == 1 ); // round to 0
static_assert( integral::div_up(   7, 5) == 2 ); // round away from 0

static_assert( integral::div_floor(-7, 5) == -2 ); // round to smaller
static_assert( integral::div_ceil( -7, 5) == -1 ); // round to larger
static_assert( integral::div_down( -7, 5) == -1 ); // round to 0
static_assert( integral::div_up(   -7, 5) == -2 ); // round away from 0
```

### Saturated math

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:93,endLineNumber:23,positionColumn:93,positionLineNumber:23,selectionStartColumn:93,selectionStartLineNumber:23,startColumn:93,startLineNumber:23),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Ausing+namespace+utl%3B%0Ausing+namespace+integral::literals%3B%0A%0A//+Helper+functions+to+avoid+ugly+casting%0Atemplate+%3Cclass+T%3E+constexpr+T+add(T+lhs,+T+rhs)+noexcept+%7B+return+lhs+%2B+rhs%3B+%7D%0Atemplate+%3Cclass+T%3E+constexpr+T+sub(T+lhs,+T+rhs)+noexcept+%7B+return+lhs+-+rhs%3B+%7D%0A%0A//+std::uint8_t+has+range+%5B0,+255%5D%0Astatic_assert(+++++++++++++++add%3Cstd::uint8_t%3E(255,+15)+%3D%3D++14+)%3B+//+overflow%0Astatic_assert(+integral::add_sat%3Cstd::uint8_t%3E(255,+15)+%3D%3D+255+)%3B+//+result+gets+clamped+to+max%0A%0A//+std::int8_t+has+range+%5B-128,+127%5D%0Astatic_assert(+++++++++++++++sub%3Cstd::int8_t%3E(-128,+14)+%3D%3D++114+)%3B+//+underflow%0A//+if+we+used+!'int!'+instead+of+!'std::int8_t!'+this+could+even+be+UB+due+to+underflow+during+signed%0A//+arithmetic+operation,+for+smaller+types+it!'s+underflow+during+cast+which+is+defined+to+wrap%0Astatic_assert(+integral::sub_sat%3Cstd::int8_t%3E(-128,+14)+%3D%3D+-128+)%3B+//+result+gets+clamped+to+min%0A%0A//+Saturated+cast%0Astatic_assert(+integral::saturate_cast%3Cstd::uint8_t%3E(++17)+%3D%3D++17+)%3B+//+regular+cast%0Astatic_assert(+integral::saturate_cast%3Cstd::uint8_t%3E(1753)+%3D%3D+255+)%3B+//+value+clamped+to+max%0Astatic_assert(+integral::saturate_cast%3Cstd::uint8_t%3E(-143)+%3D%3D+++0+)%3B+//+value+clamped+to+min%0A%0Aint+main()+%7B%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

### Heterogeneous integer comparison

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:55,endLineNumber:9,positionColumn:1,positionLineNumber:3,selectionStartColumn:55,selectionStartLineNumber:9,startColumn:1,startLineNumber:3),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Ausing+namespace+utl%3B%0A%0Astatic_assert(+std::size_t(15)+%3C+int(-7)+%3D%3D+true+)%3B%0A//+evaluates+to+!'true!'+due+to+implicit+conversion,+mathematically+incorrect+result%0A%0Astatic_assert(+integral::cmp_less(std::size_t(15),+int(-7))+%3D%3D+false+)%3B%0A//+evaluates+to+!'false!',+mathematically+correct+result%0A%0Aint+main()+%7B%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

static_assert( std::size_t(15) < int(-7) == true );
// evaluates to 'true' due to implicit conversion, mathematically incorrect result

static_assert( integral::cmp_less(std::size_t(15), int(-7)) == false );
// evaluates to 'false', mathematically correct result
```

### Narrow cast

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:5,positionColumn:1,positionLineNumber:5,selectionStartColumn:1,selectionStartLineNumber:5,startColumn:1,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Narrow+cast%0A++++%5B%5Bmaybe_unused%5D%5D+char+c1+%3D+++++++++++static_cast%3Cchar%3E(34)%3B+//+this+is+fine,+returns+34%0A++++%5B%5Bmaybe_unused%5D%5D+char+c2+%3D+integral::narrow_cast%3Cchar%3E(34)%3B+//+this+is+fine,+returns+34%0A++++%5B%5Bmaybe_unused%5D%5D+char+c3+%3D+++++++++++static_cast%3Cchar%3E(1753)%3B+//+silently+overflows,+returns+-39%0A++++%5B%5Bmaybe_unused%5D%5D+char+c4+%3D+integral::narrow_cast%3Cchar%3E(1753)%3B+//+throws+!'std::domain_error!'%0A%0A++++//+Sign+conversion%0A++++constexpr+int+N+%3D+-14%3B%0A%0A++++//+for+(std::size_t+i+%3D+0%3B+i+%3C+N%3B+%2B%2Bi)+std::cout+%3C%3C+i%3B%0A++++//+compiler+warns+about+signed/unsigned+comparison,+doesn!'t+compile+with+-Werror%0A%0A++++//+for+(std::size_t+i+%3D+0%3B+i+%3C+static_cast%3Cstd::size_t%3E(N)%3B+%2B%2Bi)+std::cout+%3C%3C+i%3B%0A++++//+casts+!'N!'+to+!'18446744073709551602!'+since+we+forgot+to+check+for+negative+!'N!'%0A%0A++++for+(std::size_t+i+%3D+0%3B+i+%3C+integral::to_unsigned(N)%3B+%2B%2Bi)+std::cout+%3C%3C+i%3B%0A++++//+this+is+good,+comparison+is+unsigned/unsigned+and+incorrect+!'N!'+will+cause+an+exception%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

// Narrow cast
[[maybe_unused]] char c1 =           static_cast<char>(34); // this is fine, returns 34
[[maybe_unused]] char c2 = integral::narrow_cast<char>(34); // this is fine, returns 34
[[maybe_unused]] char c3 =           static_cast<char>(1753); // silently overflows, returns -39
[[maybe_unused]] char c4 = integral::narrow_cast<char>(1753); // throws 'std::domain_error'

// Sign conversion
constexpr int N = -14;

// for (std::size_t i = 0; i < N; ++i) std::cout << i;
// compiler warns about signed/unsigned comparison, doesn't compile with -Werror

// for (std::size_t i = 0; i < static_cast<std::size_t>(N); ++i) std::cout << i;
// casts 'N' to '18446744073709551602' since we forgot to check for negative 'N'

for (std::size_t i = 0; i < integral::to_unsigned(N); ++i) std::cout << i;
// this is good, comparison is unsigned/unsigned and incorrect 'N' will cause an exception
```

### Integral literals

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:7,positionColumn:1,positionLineNumber:7,selectionStartColumn:1,selectionStartLineNumber:7,startColumn:1,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Ausing+namespace+utl::integral::literals%3B%0A%0A//+constexpr+auto+x+%3D+129_i8%3B%0A//+won!'t+compile,+std::int8_t+has+range+%5B-128,+127%5D%0A%0Aconstexpr+auto+x+%3D+124_i8%3B%0A//+this+is+fine,+!'x!'+has+type+!'std::int8_t!'%0A%0A//+constexpr+auto+x+%3D+-17_i8%3B%0A//+be+wary+of+this,+C%2B%2B+has+no+concept+of+signed+literals+and+treats+it+as+an+unary+minus%0A//+applied+to+!'std::int8_t!',+which+trigger+integer+promotion+and+returns+an+!'int!'%0A%0Aint+main()+%7B%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl::integral::literals;

// constexpr auto x = 129_i8;
// won't compile, std::int8_t has range [-128, 127]

constexpr auto x = 124_i8;
// this is fine, 'x' has type 'std::int8_t'

// constexpr auto x = -17_i8;
// be wary of this, C++ has no concept of signed literals and treats it as an unary minus
// applied to 'std::int8_t', which triggers integer promotion and returns an 'int'
```