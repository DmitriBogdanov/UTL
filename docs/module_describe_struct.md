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

[<- to implementation.hpp](../include/UTL/struct_reflect.hpp)

**utl::describe_reflect** is a lean `struct` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

It uses **C++17** techniques similar to [Boost.Descibe](https://github.com/boostorg/describe), but exposed in a minimal stand-alone form.

> [!Tip]
> [utl::reflect_struct]() is a **C++20** counterpart of this library, it provides the same API, but without macros and only for aggregate types.

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

## Definitions

```cpp
// Macro
#define UTL_DESCRIBE_STRUCT(name, ...)

// General reflection
template <class T> constexpr std::string_view name;
template <class T> constexpr std::size_t      size;

// Member reflection
template <std::size_t N, class T> constexpr auto label(T&& structure = T{}) noexcept;
template <std::size_t N, class T> constexpr auto value(T&& structure      ) noexcept;

// Tuple API
template <class T> auto label_view(T&& structure = T{}) noexcept; // array of labels
template <class T> auto value_view(T&& structure      ) noexcept; // tuple of values
template <class T> auto entry_view(T&& structure      ) noexcept; // tuple of label-value pairs

// Algorithms
template <class T, class F> void for_indices(F&& f); // f = f(i)
```

## Methods

### Macros

> ```cpp
> #define UTL_STRUCT_REFLECT(struct_name, ...)
> ```

Registers reflection for the `struct` / `class` type `struct_name` with member variables `...`.

### Reflection

> ```cpp
> template <class S> constexpr std::string_view type_name;
> ```

Evaluates to a stringified name of struct `S`.

> ```cpp
> template <class S> constexpr std::size_t size;
> ```

Evaluates to a number of fields in the struct `S` .

> ```cpp
> template <class S> constexpr std::array<std::string_view, size<E>> names;
> ```

Evaluates to an array of stringified field names corresponding to struct `S`.

> ```cpp
> template <class S> constexpr auto field_view(S&& value) noexcept;
> ```

Returns a tuple with perfectly-forwarded references corresponding to the fields of `value`.

Below is an **example table** for the reflection of `struct Struct { int x; };`:

| Value category                              | Forwarded reference                   | `field_view` return type |
| ------------------------------------------- | ------------------------------------- | ------------------------ |
| `value` is a const reference to a struct    | `S&&` corresponds to `const Struct&`  | `std:tuple<const int&>`  |
| `value` is an l-value reference to a struct | `S&&` corresponds to `Struct&`        | `std:tuple<int&>`        |
| `value` is an r-value reference to a struct | `S&&` corresponds to `Struct&&`       | `std:tuple<int&&>`       |

> [!Tip]
> This effectively means that `field_view` allows struct members to be accessed exactly as one would expect when working with struct members directly, except using a tuple API. See the [examples](#field--entry-views).

> ```cpp
> template <class S> constexpr auto entry_view(S&& value) noexcept;
> ```

Returns a tuple with pairs of names and perfectly-forwarded references corresponding to the fields of `value`.

Reference forwarding logic is exactly the same as it is in `field_view()`. Below is an **example table** for the reflection of `struct Struct { int x; };`:

| Value category                              | Forwarded reference                   | `entry_view()` return type                           |
| ------------------------------------------- | ------------------------------------- | ---------------------------------------------------- |
| `value` is a const reference to a struct    | `S&&` corresponds to `const Struct&`  | `std:tuple<std::pair<std::string_view, const int&>>` |
| `value` is an l-value reference to a struct | `S&&` corresponds to `Struct&`        | `std:tuple<std::pair<std::string_view, int&>>`       |
| `value` is an r-value reference to a struct | `S&&` corresponds to `Struct&&`       | `std:tuple<std::pair<std::string_view, int&&>>`      |

> ```cpp
> template <std::size_t I, class S> constexpr auto get(S&& value) noexcept;
> ```

Returns perfectly-forwarded reference to the field number `I` in `value`.

> ```cpp
> template <class S, class Func>
> constexpr void for_each(S&& value, Func&& func);
> ```

Applies function `func` to all fields of the struct `value`.

**Note:** `func` must be callable for all field types, either through overloads or templating.

> ```cpp
> template <class S1, class S2, class Func>
> constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func);
> ```

Applies function `func` to all fields of a struct pair `value_1`, `value_2`.

**Note:** This is useful for defining binary functions over custom types, see the [examples](#using-reflection-to-define-binary-operations).

> ```cpp
> template <class S, class Pred>
> constexpr bool true_for_all(const S& value, Pred&& pred);
> ```

Returns whether unary predicate `pred` is satisfied for all fields of the `value`.

**Note:** Predicate checks cannot be efficiently implemented in terms of `for_each()` due to potential short-circuiting of logical AND. Use this function instead.

> ```cpp
> template <class S1, class S2, class Pred>
> constexpr bool true_for_all(const S1& value_1, const S2& value_2, Pred&& pred);
> ```

Returns whether binary predicate `pred` is satisfied for all fields of a struct pair `value_1`, `value_2`.

### Other utils

> ```cpp
> template <class T, class Func>
> constexpr void tuple_for_each(T&& tuple, Func&& func)
> ```

Applies unary function `func` to all elements of the tuple `tuple`.

**Note:** This is not a part reflection, the function is provided for convenience when working with tuples in general.

> ```cpp
> template <class T1, class T2, class Func>
> constexpr void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func)
> ```

Applies binary function `func` to all elements of the tuple pair `tuple_1`, `tuple_2`.

**Note:** This is not a part reflection, the function is provided for convenience when working with tuples in general.

## Examples

### Basics

[ [Run this code](https://godbolt.org/z/e5qq7eb11) ] [ [Open source file](../examples/module_struct_reflect/basic_reflection.cpp) ]

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
```

### Tuple API

[ [Run this code](https://godbolt.org/z/oz1zPY95f) ] [ [Open source file](../examples/module_struct_reflect/field_and_entry_views.cpp) ]

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

using namespace std::string_view_literals;

static_assert( std::get<0>(describe_struct::entry_view(vec)) ==  std::pair{ "x"sv, 1. } );
static_assert( std::get<1>(describe_struct::entry_view(vec)) ==  std::pair{ "y"sv, 2. } );
static_assert( std::get<2>(describe_struct::entry_view(vec)) ==  std::pair{ "z"sv, 3. } );
```

### Serialization

[ [Run this code](https://godbolt.org/z/aWMeKx1sx) ] [ [Open source file](../examples/module_struct_reflect/using_reflection_to_define_binary_operations.cpp) ]

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

### Member-wise operator

[ [Run this code](https://godbolt.org/z/o8noxx6P6) ] [ [Open source file](../examples/module_struct_reflect/iterating_over_a_generic_tuple.cpp) ]

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
