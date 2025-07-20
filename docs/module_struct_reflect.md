[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::struct_reflect

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/struct_reflect.hpp)

**utl::struct_reflect** is a lean `struct` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

## Definitions

```cpp
// Macros
#define UTL_STRUCT_REFLECT(struct_name, ...)

// Reflection
template <class S> constexpr std::string_view type_name;
template <class S> constexpr std::size_t      size;

template <class S> constexpr std::array<std::string_view, size<E>> names;
template <class S> constexpr auto field_view(S&& value) noexcept;
template <class S> constexpr auto entry_view(S&& value) noexcept;

template <std::size_t I, class S> constexpr auto get(S&& value) noexcept;

template <class S, class Func>
constexpr void for_each(S&& value, Func&& func);

template <class S1, class S2, class Func>
constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func);

template <class S, class Pred>
constexpr bool true_for_all(const S& value, Pred&& pred);

template <class S1, class S2, class Pred>
constexpr bool true_for_all(const S1& value_1, const S2& value_2, Pred&& pred);

// Other utils
template <class T, class Func>
void tuple_for_each(T&& tuple, Func&& func);

template <class T1, class T2, class Func>
void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func);
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
> This effectively means that `field_view` allows struct members to be accessed exactly as one would expect when working with struct members directly, except using a tuple API. See [examples]().

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

**Note:** This is useful for defining binary functions over custom types, see [examples]().

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

### Basic reflection

[ [Run this code](https://godbolt.org/z/e5qq7eb11) ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test basic reflection
using namespace utl;

static_assert( struct_reflect::type_name<Quaternion> == "Quaternion" );

static_assert( struct_reflect::size<Quaternion> == 4 );

static_assert( struct_reflect::names<Quaternion>[0] == "r" );
static_assert( struct_reflect::names<Quaternion>[1] == "i" );
static_assert( struct_reflect::names<Quaternion>[2] == "j" );
static_assert( struct_reflect::names<Quaternion>[3] == "k" );

constexpr Quaternion q = { 5., 6., 7., 8. };

static_assert( struct_reflect::get<0>(q) == 5. );
static_assert( struct_reflect::get<1>(q) == 6. );
static_assert( struct_reflect::get<2>(q) == 7. );
static_assert( struct_reflect::get<3>(q) == 8. );
```

### Field & entry views

[ [Run this code](https://godbolt.org/z/oz1zPY95f) ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test field & entry views
using namespace utl;

constexpr Quaternion q = { 5., 6., 7., 8. };

static_assert( struct_reflect::field_view(q) == std::tuple{ 5., 6., 7., 8. } );

static_assert( std::get<0>(struct_reflect::entry_view(q)).first  == "r" );
static_assert( std::get<0>(struct_reflect::entry_view(q)).second == 5.  );
static_assert( std::get<1>(struct_reflect::entry_view(q)).first  == "i" );
static_assert( std::get<1>(struct_reflect::entry_view(q)).second == 6.  );
static_assert( std::get<2>(struct_reflect::entry_view(q)).first  == "j" );
static_assert( std::get<2>(struct_reflect::entry_view(q)).second == 7.  );
static_assert( std::get<3>(struct_reflect::entry_view(q)).first  == "k" );
static_assert( std::get<3>(struct_reflect::entry_view(q)).second == 8.  );
```

### Using reflection to define binary operations

[ [Run this code](https://godbolt.org/z/aWMeKx1sx) ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Define binary operation (member-wise addition)
constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    Quaternion res = lhs;
    utl::struct_reflect::for_each(res, rhs, [&](auto& l, const auto& r){ l += r; });
    return res;
}

// Define binary operation with predicates (member-wise equality)
constexpr bool operator==(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    return utl::struct_reflect::true_for_all(lhs, rhs, [&](const auto& l, const auto& r){ return l == r; });
}

// Test operations
static_assert( Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} == Quaternion{6, 8, 10, 12} );
```

### Iterating over a generic tuple

[ [Run this code](https://godbolt.org/z/o8noxx6P6) ]

```cpp
using namespace utl;

std::tuple<std::string, int   > tuple_1{ "lorem", 2 };
std::tuple<const char*, double> tuple_2{ "ipsum", 3 };

// Print tuple
struct_reflect::tuple_for_each(tuple_1, [&](auto&& x){ std::cout << x << '\n'; });

// Print tuple sum
struct_reflect::tuple_for_each(tuple_1, tuple_2, [&](auto&& x, auto&& y){ std::cout << x + y << '\n'; });

// notice that tuples don't have to be homogenous,
// what matters is that binary function can be called on all corresponding pairs
```

Output:

```
lorem
2
loremipsum
5
```

### Debug printing with `utl::log`

[ [Run this code](https://godbolt.org/z/h3h8f3KWW) ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// ...

// Print struct
using namespace utl;

constexpr Quaternion q = { 0.5, 1.5, 2.5, 3.5 };

log::println("q = ", struct_reflect::entry_view(q));

// Note: there is no tight coupling between the modules, 
//       'utl::log' just knows how to expand tuples,
//       other loggers that do this will also work
```

Output:

```
q = < < r, 0.5 >, < i, 1.5 >, < j, 2.5 >, < k, 3.5 > >
```