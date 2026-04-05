[<img src ="images/badge_language_cpp_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](guide_license.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_docs.svg">](https://dmitribogdanov.github.io/UTL/)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_workflow_windows.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_workflow_ubuntu.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_workflow_macos.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)
[<img src ="images/badge_workflow_freebsd.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/freebsd.yml)

# utl::describe_enum

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/describe_enum.hpp)

**utl::describe_enum** is a lean `enum` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

## Definitions

```cpp
// Macros
#define UTL_DESCRIBE_ENUM(enum_name, ...)

// Reflection
template <class E> constexpr std::string_view name;
template <class E> constexpr std::size_t      size;

template <class E> constexpr std::array<          std::string_view    , size<E>> label_array;
template <class E> constexpr std::array<                            E , size<E>> value_array;
template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entry_array;

template <class E> constexpr bool is_valid     (E value) noexcept;
template <class E> constexpr auto to_underlying(E value) noexcept;

template <class E> constexpr std::string_view   to_string(               E  value);
template <class E> constexpr E                from_string(std::string_view string);
```

## Methods

### Macros

> ```cpp
> #define UTL_DESCRIBE_ENUM(enum_name, ...)
> ```

Registers reflection for the `enum` / `enum class` type `enum_name` with elements `...`.

### Reflection

> ```cpp
> template <class E> constexpr std::string_view name;
> ```

Evaluates to stringified name of `E` enum.

> ```cpp
> template <class E> constexpr std::size_t size;
> ```

Evaluates to a number of elements in `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::string_view, size<E>> label_array;
> ```

Evaluates to an array of stringified element names corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<E, size<E>> value_array;
> ```

Evaluates to an array of elements corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entry_array;
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

[ [Run this code]() ] [ [Open source file](../examples/module_describe_enum/reflecting_an_enum.cpp) ]

```cpp
// Register enum & reflection
enum class side { left = -1, right = 1, none = 0 };

UTL_DESCRIBE_ENUM(side, left, right, none);

// Test reflection
using namespace utl;
using namespace std::string_view_literals;

static_assert( describe_enum::name<side> == "side" );

static_assert( describe_enum::size<side> == 3 );

static_assert( describe_enum::label_array<side>[0] == "left"  );
static_assert( describe_enum::label_array<side>[1] == "right" );
static_assert( describe_enum::label_array<side>[2] == "none"  );

static_assert( describe_enum::value_array<side>[0] == side::left  );
static_assert( describe_enum::value_array<side>[1] == side::right );
static_assert( describe_enum::value_array<side>[2] == side::none  );

static_assert( describe_enum::entry_array<side>[0]  == std::pair{  "left"sv, side::left  } );
static_assert( describe_enum::entry_array<side>[1]  == std::pair{ "right"sv, side::right } );
static_assert( describe_enum::entry_array<side>[2]  == std::pair{  "none"sv, side::none  } );

static_assert( describe_enum::is_valid(side{-1}) == true  );
static_assert( describe_enum::is_valid(side{ 1}) == true  );
static_assert( describe_enum::is_valid(side{ 0}) == true  );
static_assert( describe_enum::is_valid(side{ 2}) == false );

static_assert( describe_enum::to_underlying(side::left ) == -1 );
static_assert( describe_enum::to_underlying(side::right) ==  1 );
static_assert( describe_enum::to_underlying(side::none ) ==  0 );

static_assert( describe_enum::to_string(side::left ) == "left"  );
static_assert( describe_enum::to_string(side::right) == "right" );
static_assert( describe_enum::to_string(side::none ) == "none"  );

static_assert( describe_enum::from_string<side>("left" ) == side::left  );
static_assert( describe_enum::from_string<side>("right") == side::right );
static_assert( describe_enum::from_string<side>("none" ) == side::none  );
```