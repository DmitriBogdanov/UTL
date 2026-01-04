[<img src ="images/badge_language_cpp_20.svg">](https://en.cppreference.com/w/cpp/20.html)
[<img src ="images/badge_license_mit.svg">](LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_docs.svg">](https://dmitribogdanov.github.io/UTL/)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_workflow_windows.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_workflow_ubuntu.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_workflow_macos.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)
[<img src ="images/badge_workflow_freebsd.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/freebsd.yml)

# utl::reflect_struct

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/reflect_struct.hpp)

**utl::reflect_struct** is a lean library for reflecting non-derived aggregate structures.

It uses **C++20** techniques similar to [Glaze](https://github.com/stephenberry/glaze), [reflect-cpp](https://github.com/getml/reflect-cpp) and [Boost.PFR](https://github.com/boostorg/pfr), but exposed in a minimal stand-alone form.

The goal is to have a clean well-documented implementation that is easy to analyze and integrate with existing projects.

## Definitions

```cpp
// Restrictions
template <class T> concept reflectable = /* non-derived, aggregate and below member limit (20) */;

// General reflection
template <class T> constexpr std::string_view name;
template <class T> constexpr std::size_t      size;

// Member reflection
template <std::size_t N, class T> constexpr auto label(T&& structure = T{}) noexcept;
template <std::size_t N, class T> constexpr auto value(T&& structure      ) noexcept;
template <std::size_t N, class T> constexpr auto entry(T&& structure      ) noexcept;

// Tuple API
template <class T> auto label_view(T&& structure = T{}) noexcept; // array of labels
template <class T> auto value_view(T&& structure      ) noexcept; // tuple of values
template <class T> auto entry_view(T&& structure      ) noexcept; // tuple of label-value pairs

// Algorithms
template <class T, class F> void for_indices(F&& f); // f = f(i)
```

> [!Note]
> All templates have appropriate concept constraints, which are omitted here to reduce verbosity.

## Methods

### Restrictions

> ```cpp
> template <class T> concept reflectable = /* non-derived, aggregate and below member limit (20) */;
> ```

Concept satisfied for types that can be reflected by this library.

To be `reflectable` class `T` needs to be an [aggregate type](https://en.cppreference.com/w/cpp/language/aggregate_initialization.html) with no base classes and no more than 20 member variables.

### General reflection

> ```cpp
> template <class T> constexpr std::string_view name;
> ```

Evaluates to a stringified name of the struct `T`.

> ```cpp
> template <class T> constexpr std::size_t size;
> ```

Evaluates to a number of member variables in the struct `T` .

### Member reflection

> ```cpp
> template <std::size_t N, class T> constexpr auto label(T&& structure = T{}) noexcept;
> ```

Evaluates to the [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view.html) label of the Nth member variable of `T`.

Providing specific `structure` is not necessary, but can be done to deduce the template.

> ```cpp
> template <std::size_t N, class T> constexpr auto value(T&& structure) noexcept;
> ```

Returns a perfectly-forwarded reference to the Nth member variable of `structure`.

Below is an **example table** for the reflection of `struct mystruct { int x; };`:

| Value category                                  | Forwarded reference                    | `value<0>` return type |
| ----------------------------------------------- | -------------------------------------- | ---------------------- |
| `structure` is a const reference to a struct    | `T&&` corresponds to `const mystruct&` | `const int&`           |
| `structure` is an l-value reference to a struct | `T&&` corresponds to `mystruct&`       | `int&`                 |
| `structure` is an r-value reference to a struct | `T&&` corresponds to `mystruct&&`      | `int&&`                |

> ```cpp
> template <std::size_t N, class T> constexpr auto entry(T&& structure) noexcept;
> ```

Returns [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair.html) with label and a perfectly-forwarded reference to the Nth member variable of `structure`.

Below is an **example table** for the reflection of `struct mystruct { int x; };`:

| Value category                                  | Forwarded reference                    | `entry<0>` return type                    |
| ----------------------------------------------- | -------------------------------------- | ----------------------------------------- |
| `structure` is a const reference to a struct    | `T&&` corresponds to `const mystruct&` | `std::pair<std::string_view, const int&>` |
| `structure` is an l-value reference to a struct | `T&&` corresponds to `mystruct&`       | `std::pair<std::string_view, int&>`       |
| `structure` is an r-value reference to a struct | `T&&` corresponds to `mystruct&&`      | `std::pair<std::string_view, int&&>`      |

### Tuple API

> ```cpp
> template <class T> auto label_view(T&& structure = T{}) noexcept; // array of labels
> ```

Returns an [`std::array`](https://en.cppreference.com/w/cpp/container/array.html) of [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view.html) labels corresponding to member variables of `structure`.

> ```cpp
> template <class T> auto value_view(T&& structure) noexcept; // tuple of values
> ```

Returns a tuple of perfectly-forwarded references to the member variables of `structure`.

Below is an **example table** for the reflection of `struct mystruct { int x; };`:

| Value category                                  | Forwarded reference                    | `value_view` return type |
| ----------------------------------------------- | -------------------------------------- | ------------------------ |
| `structure` is a const reference to a struct    | `T&&` corresponds to `const mystruct&` | `std:tuple<const int&>`  |
| `structure` is an l-value reference to a struct | `T&&` corresponds to `mystruct&`       | `std:tuple<int&>`        |
| `structure` is an r-value reference to a struct | `T&&` corresponds to `mystruct&&`      | `std:tuple<int&&>`       |

> ```cpp
> template <class T> auto entry_view(T&& structure) noexcept; // tuple of label-value pairs
> ```

Returns a tuple of label-value pairs corresponding to member variables of `structure`.

Similarly to `value_view()`, value references are propagated with perfect forwarding.

Below is an **example table** for the reflection of `struct mystruct { int x; };`:

| Value category                                  | Forwarded reference                    | `entry_view` return type                             |
| ----------------------------------------------- | -------------------------------------- | ---------------------------------------------------- |
| `structure` is a const reference to a struct    | `T&&` corresponds to `const mystruct&` | `std:tuple<std::pair<std::string_view, const int&>>` |
| `structure` is an l-value reference to a struct | `T&&` corresponds to `mystruct&`       | `std:tuple<std::pair<std::string_view, int&>>`       |
| `structure` is an r-value reference to a struct | `T&&` corresponds to `mystruct&&`      | `std:tuple<std::pair<std::string_view, int&&>>`      |

### Algorithms

> ```cpp
> template <class T, class F> void for_indices(F&& f); // f = f(i)
> ```

Invokes [`f(std::integral_constant<std::size_t, i>)`](https://en.cppreference.com/w/cpp/types/integral_constant.html) for indices `0` to `size<T> - 1`.

If none of the `f` invocations are throwing, this function will be marked `noexcept`.

In practice this is usually used with a template lambda.

## Examples

### Basics

[ [Run this code]() ] [ [Open source file](../examples/module_reflect_struct/basics.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; };

// General reflection
static_assert( reflect_struct::name<vector> == "vector" );
static_assert( reflect_struct::size<vector> == 3        );

// Member reflection
constexpr vector vec = { 1., 2., 3. };

static_assert( reflect_struct::label<0>(vec) == "x" );
static_assert( reflect_struct::label<1>(vec) == "y" );
static_assert( reflect_struct::label<2>(vec) == "z" );

static_assert( reflect_struct::value<0>(vec) ==  1. );
static_assert( reflect_struct::value<1>(vec) ==  2. );
static_assert( reflect_struct::value<2>(vec) ==  3. );

static_assert( reflect_struct::entry<0>(vec) == std::pair{ "x", 1. } );
static_assert( reflect_struct::entry<1>(vec) == std::pair{ "y", 2. } );
static_assert( reflect_struct::entry<2>(vec) == std::pair{ "z", 3. } );
```

### Tuple API

[ [Run this code]() ] [ [Open source file](../examples/module_reflect_struct/tuple_api.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; };

constexpr vector vec = { 1., 2., 3. };

// Access it like a tuple
static_assert( std::get<0>(reflect_struct::label_view(vec)) == "x" );
static_assert( std::get<1>(reflect_struct::label_view(vec)) == "y" );
static_assert( std::get<2>(reflect_struct::label_view(vec)) == "z" );

static_assert( std::get<0>(reflect_struct::value_view(vec)) ==  1. );
static_assert( std::get<1>(reflect_struct::value_view(vec)) ==  2. );
static_assert( std::get<2>(reflect_struct::value_view(vec)) ==  3. );

static_assert( std::get<0>(reflect_struct::entry_view(vec)) ==  std::pair{ "x", 1. } );
static_assert( std::get<1>(reflect_struct::entry_view(vec)) ==  std::pair{ "y", 2. } );
static_assert( std::get<2>(reflect_struct::entry_view(vec)) ==  std::pair{ "z", 3. } );
```

### Serialization

[ [Run this code]() ] [ [Open source file](../examples/module_reflect_struct/serialization.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; };

constexpr vector vec = { 0.5, 1.5, 2.5 };

// Serialize members
reflect_struct::for_indices<vector>([&] (auto i) {
    std::cout << "[" << i << "]: " << reflect_struct::label<i>(vec) << " -> " << reflect_struct::value<i>(vec) << "\n";
});
```

Output:

```
[0]: x -> 0.5
[1]: y -> 1.5
[2]: z -> 2.5
```

### Operators

[ [Run this code]() ] [ [Open source file](../examples/module_reflect_struct/operators.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; };

// Member-wise 'operator+'
constexpr vector operator+(vector lhs, vector rhs) noexcept {
    vector res;

    reflect_struct::for_indices<vector>([&] (auto i) {
        reflect_struct::value<i>(res) = reflect_struct::value<i>(lhs) + reflect_struct::value<i>(rhs);
    });

    return res;
}

// Member-wise 'operator=='
constexpr bool operator==(vector lhs, vector rhs) noexcept {
    bool res = true;

    reflect_struct::for_indices<vector>([&] (auto i) {
        if (reflect_struct::value<i>(lhs) != reflect_struct::value<i>(rhs)) res = false;
    });

    return res;
}

static_assert( vector{1, 2, 3} + vector{4, 5, 6} == vector{5, 7, 9} );
```
