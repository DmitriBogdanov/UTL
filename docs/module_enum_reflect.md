[<img src ="images/badge_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](../LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_windows_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_ubuntu_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_macos_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::enum_reflect

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/enum_reflect.hpp)

**utl::enum_reflect** is a lean `enum` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

## Definitions

```cpp
// Macros
#define UTL_ENUM_REFLECT(enum_name, ...)

// Reflection
template <class E> constexpr std::string_view type_name;
template <class E> constexpr std::size_t      size;

template <class E> constexpr std::array<std::string_view, size<E>>               names;
template <class E> constexpr std::array<E, size<E>>                              values;
template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entries;

template <class E> constexpr bool      is_valid(E value) noexcept;
template <class E> constexpr auto to_underlying(E value) noexcept;

template <class E> constexpr std::string_view   to_string(E value);
template <class E> constexpr E                from_string(std::string_view str);
```

## Methods

### Macros

> ```cpp
> #define UTL_ENUM_REFLECT(enum_name, ...)
> ```

Registers reflection for the `enum` / `enum class` type `enum_name` with elements `...`.

### Reflection

> ```cpp
> template <class E> constexpr std::string_view type_name;
> ```

Evaluates to stringified name of `E` enum.

> ```cpp
> template <class E> constexpr std::size_t size;
> ```

Evaluates to a number of elements in `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::string_view, size<E>> names;
> ```

Evaluates to an array of stringified element names corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<E, size<E>> values;
> ```

Evaluates to an array of elements corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entries;
> ```

Evaluates to an array of name-value pairs corresponding to `E` enum.

> ```cpp
> template <class E> constexpr bool is_valid(E value) noexcept;
> ```

Returns whether enum-typed `value` is a valid element of `E` enum. See [examples](#reflecting-an-enum).

> ```cpp
> template <class E> constexpr auto to_underlying(E value) noexcept;
> ```

Equivalent to `static_cast<std::underlying_type_t<E>>(value)`. In C++23 can be replaced with [`std::to_underlying()`](https://en.cppreference.com/w/cpp/utility/to_underlying).

**Note:** This particular function is included for convenience and does not require `E` to be reflected.

> ```cpp
> template <class E> constexpr std::string_view to_string(E value);
> ```

Returns string corresponding to a `value` from `E` enum.

Throws [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) if `value` is not a part of enum.

> ```cpp
> template <class E> constexpr E from_string(std::string_view str);
> ```

Returns value from `E` enum corresponding to a string `str`.

Throws [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) if `str` does not correspond to any element of the enum.

## Examples

### Reflecting an enum

[ [Run this code](https://godbolt.org/z/bq9bv8jr5) ]

```cpp
// Register enum & reflection
enum class Side { LEFT = -1, RIGHT = 1, NONE = 0 };

UTL_ENUM_REFLECT(Side, LEFT, RIGHT, NONE);

// Test reflection
using namespace utl;
using namespace std::string_view_literals;

static_assert( enum_reflect::type_name<Side> == "Side" );

static_assert( enum_reflect::size<Side> == 3 );

static_assert( enum_reflect::names<Side>[0] == "LEFT"  );
static_assert( enum_reflect::names<Side>[1] == "RIGHT" );
static_assert( enum_reflect::names<Side>[2] == "NONE"  );

static_assert( enum_reflect::values<Side>[0] == Side::LEFT  );
static_assert( enum_reflect::values<Side>[1] == Side::RIGHT );
static_assert( enum_reflect::values<Side>[2] == Side::NONE  );

static_assert( enum_reflect::entries<Side>[0]  == std::pair{  "LEFT"sv, Side::LEFT  } );
static_assert( enum_reflect::entries<Side>[1]  == std::pair{ "RIGHT"sv, Side::RIGHT } );
static_assert( enum_reflect::entries<Side>[2]  == std::pair{  "NONE"sv, Side::NONE  } );

static_assert( enum_reflect::is_valid(Side{-1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 0}) == true  );
static_assert( enum_reflect::is_valid(Side{ 2}) == false );

static_assert( enum_reflect::to_underlying(Side::LEFT ) == -1 );
static_assert( enum_reflect::to_underlying(Side::RIGHT) ==  1 );
static_assert( enum_reflect::to_underlying(Side::NONE ) ==  0 );

static_assert( enum_reflect::to_string(Side::LEFT ) == "LEFT"  );
static_assert( enum_reflect::to_string(Side::RIGHT) == "RIGHT" );
static_assert( enum_reflect::to_string(Side::NONE ) == "NONE"  );

static_assert( enum_reflect::from_string<Side>("LEFT" ) == Side::LEFT  );
static_assert( enum_reflect::from_string<Side>("RIGHT") == Side::RIGHT );
static_assert( enum_reflect::from_string<Side>("NONE" ) == Side::NONE  );
```