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

# utl::describe_struct

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/describe_struct.hpp)

**utl::describe_struct** is a lean class reflection library based around the [map-macro](https://github.com/swansontec/map-macro) annotations.

It uses **C++17** techniques similar to [Boost.Describe](https://github.com/boostorg/describe), but exposed in a minimal stand-alone form.

The goal is to have a clean well-documented implementation that is easy to analyze and integrate with existing projects.

> [!Tip]
> See [utl::reflect_struct](module_reflect_struct.md) for a **C++20** counterpart of this library, it provides the exact same API, but without requiring macro annotations.

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

## Definitions

```cpp
// Macro annotation
#define UTL_DESCRIBE_STRUCT(name, ...)

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

## Methods

### Macro annotation

> ```cpp
> #define UTL_DESCRIBE_STRUCT(name, ...)
> ```

Registers reflection for the `struct` / `class` type `name` with member variables `...`.

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

[ [Run this code](https://godbolt.org/z/e5qq7eb11) ] [ [Open source file](../examples/module_describe_struct/basics.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

// General reflection
static_assert( describe_struct::name<vector> == "vector" );
static_assert( describe_struct::size<vector> == 3        );

// Member reflection
constexpr vector vec = { 1., 2., 3. };

static_assert( describe_struct::label<0>(vec) == "x" );
static_assert( describe_struct::label<1>(vec) == "y" );
static_assert( describe_struct::label<2>(vec) == "z" );

static_assert( describe_struct::value<0>(vec) ==  1. );
static_assert( describe_struct::value<1>(vec) ==  2. );
static_assert( describe_struct::value<2>(vec) ==  3. );

static_assert( describe_struct::entry<0>(vec).first  == "x" );
static_assert( describe_struct::entry<1>(vec).first  == "y" );
static_assert( describe_struct::entry<2>(vec).first  == "z" );

static_assert( describe_struct::entry<0>(vec).second ==  1. );
static_assert( describe_struct::entry<1>(vec).second ==  2. );
static_assert( describe_struct::entry<2>(vec).second ==  3. );
```

### Tuple API

[ [Run this code](https://godbolt.org/z/oz1zPY95f) ] [ [Open source file](../examples/module_describe_struct/tuple_api.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

constexpr vector vec = { 1., 2., 3. };

// Access it like a tuple
static_assert( std::get<0>(describe_struct::label_view(vec)) == "x" );
static_assert( std::get<1>(describe_struct::label_view(vec)) == "y" );
static_assert( std::get<2>(describe_struct::label_view(vec)) == "z" );

static_assert( std::get<0>(describe_struct::value_view(vec)) ==  1. );
static_assert( std::get<1>(describe_struct::value_view(vec)) ==  2. );
static_assert( std::get<2>(describe_struct::value_view(vec)) ==  3. );

static_assert( std::get<0>(describe_struct::entry_view(vec)).first  == "x" );
static_assert( std::get<1>(describe_struct::entry_view(vec)).first  == "y" );
static_assert( std::get<2>(describe_struct::entry_view(vec)).first  == "z" );

static_assert( std::get<0>(describe_struct::entry_view(vec)).second ==  1. );
static_assert( std::get<1>(describe_struct::entry_view(vec)).second ==  2. );
static_assert( std::get<2>(describe_struct::entry_view(vec)).second ==  3. );
```

### Serialization

[ [Run this code](https://godbolt.org/z/aWMeKx1sx) ] [ [Open source file](../examples/module_describe_struct/serialization.cpp) ]

```cpp
using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

constexpr vector vec = { 0.5, 1.5, 2.5 };

// Serialize members
describe_struct::for_each(vec, [] (auto i) {
   std::cout << "[" << i << "]: " << describe_struct::label<i>(vec) << " -> " << describe_struct::value<i>(vec);
});
```

Output:

```
[0]: x -> 0.5
[1]: y -> 1.5
[2]: z -> 2.5
```

### Operators

[ [Run this code](https://godbolt.org/z/o8noxx6P6) ] [ [Open source file](../examples/module_describe_struct/operators.cpp) ]

```cpp
using namespace utl;

// Define structure
struct quaternion { double i, j, k, r; }; UTL_DESCRIBE_STRUCT(quaternion, i, j, k, r);

// Member-wise 'operator+'
constexpr quaternion operator+(quaternion lhs, quaternion rhs) noexcept {
    quaternion res;

    describe_struct::for_each(res, [] (auto i) {
        describe_struct::value<i>(res) = describe_struct::value<i>(lhs) + describe_struct::value<i>(rhs);
    });

    return res;
}

static_assert( quaternion{1, 2, 3, 4} + quaternion{4, 3, 2, 1} == quaternion{5, 5, 5, 5} );
```
