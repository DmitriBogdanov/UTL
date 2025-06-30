[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::enum_reflect

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/enum_reflect.hpp)

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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAA7KQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S5yAAqAUuLCY42ilxSRnoAH1trtMAjkQ8EAkEodsCYNABBak0657ABKmGeGOIe0YPgOZgAbHtiJgqG00vSuSw9jt0Qo9kI8PsTLELHsAtgAGKIg4bQF7AC0zWZAEkAOIACU1hx1BtCAHlQtgtTqNAdYoDDlZafSkQFcdhQnIALK4pnq1VuREQOVYUgq9WImNMk3mmO2%2B1zd30%2BnBAh7NHBCBzF0eml7Ut7RmIzDjAVCkUCellvYfYLAPbMNgKBJMVZNgi0DO0xvNoxt1hVrs98boEAgcZgoy42F4TB3XG0QjRMQKAd0wdl8ZMRzIXHS6IECCchg%2BXGC4UVAgzggATwSmFx7cwhzcUc/GwdlstHkzB/cwzD2dMNmLBt9xqI8TwUT5z0va9bzaGcUgAL1/b95V/f9tUAjZwJ3aDSwPODTzqC9xRvWt7xnD9tyOEC/xMABWCwNHYnUAO1IDVQ1UDSwg4tG3IvBj0opCaNQ%2BiQEYr8WKpDiuG4x1ANAxMzURISRNIvZxMkhCz2oq8WFou8HAYscmJwrBKXYiwzDU3ieLMMxUypdzhJIvcyNgiT4MQ0yULoqyQFhMQvCrRTcIcjiuLYniCL4n8ZwEzViMg/TDKCkzkPM2Twsi7wYuYuLWJUlyUp1NKQC080stEmDD0CqSQsKsKHwiqKyrsvDHOcpL1NS3CZ08nzss9PyDICozgoKiy0JAH4wT6pTHMSnURp1KcZy7PBiEVZV%2BLjUCFFhGM6oy0tFR1PSZty9rFqK7rVuXWyNqq4adtm6cQAOo6lSAhqdPci6rrG%2BqkwtV0mpyua8qol6upnd71oq5SnLU369oBphDuO273M887LtlKGJpde7fJLFqKOM5GZNRkA8AURcxHlSM4qVfU7sLVy9gIYhosm5r/Na%2Bb8uZyzurZjn13Qbn7OB1TXQFmqhZFzAxYRyWkeksylrk%2BWSq5kDga49XfuF0X4cexHnpl5bTc5pWLeVZzrcFqgtx1h7dzpiWGYW525KIXEvAYLBiFoJ8W2V9gQBujW3T4/V7aD2b9ado3XsfVBI%2Bj6I44TurQdTwC9i4TOxMdxnDdC2WC6LmPS6MRPxrtB1K74vZnQDvWQ%2BlvOWYjucy6hlPftAjKhNr%2Bm2objrjfC8fhcnrAZwrmf3NB3SdzrnPl5R5uQHX%2BdgE7kAJt7tyPO7%2BfB4d4/Q9Hs%2BqGIVBzInoxYvsv8EBZ5nW8nfCmW9k5xl1i/YeTN37LU/t/XEv9gD/zwkAveMNQJgPLjDBewcl5vybggr%2BP8N5/3KgA7AGCH72l0r9OqVNB6unpBwBYtBOBsV4H4DgWhSCoE4G4aw1gDJLBWDrTYPBSAPl4WwhYABrEAvI2IPA2LEAAHGxAAnBoDQGiNhmHUVo9R%2BhOCSG4ZoXgAiOC8AuBoaRliFhwFgDARAKBv4JDoNEcglB/iePoDEKURguC8l0XwOgBBogXAgBESxpAIjBFqE%2BTgUiEnMGIE%2BG0ERtD0W4Lwf4bBBA2gYHHOJWB3jADcGIWgFw8mkCwGiHEaw%2BH4EFJUWEVY4mYFUBULwkS4nZlaHE9cERbgZI8FvWR0iwQsBSbwDpxAIjJEwICTAjTgDriMI4vgBhgAKAAGrLjuDaV8PCpH8EECIMQ7ApAyEEIoFQ6gpm6GaAYLZphLDWH0HgCIFxIALFQAkdotTdRTktB8qwlguCxD1DaMwViFlgiwH8gsLQ6x%2BAgK4EYTRSCBCmEUEo2RkipAENioluQ0g9AJf0MYrR7xVAmGS2l6LOh1CpX0GIYxGWeEaHoA8bL8UcokAsBQYjVjCtMRwLhpAeF8OsXsVQ6jeS6l5JISUbzWwhIeBobVewIC4EICQHkGwuBzF4DIrQcwFgIEwEwGOlB2FmN4LMti9jZVWM4LYkA9iLVsNIM4txSwzh9J8RAPxXjiChDHJwRVyrVXqsMJq3k2rtW8EwPgIgSK9AXOEKIcQtyc0PLUHEl5pA7i3ASHMx1UqLFTOsTaPpCQ%2Bl7FQFQBVSqVVqqCUmlNzoIAeBYP46IxrTXmscQokAkhk28i0ZIMwXAlUaG0bELgITJXmNIC6t1cTrFep9eO0giiNiSAeLyedkgNBaK0fO3kK6NBmGkBwjgGxa1ys9Q42RVrJXwplTu99vqv0LJSM4SQQA%3D%3D%3D) ]

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