# Map-macro: emulating reflection

Recently I've come across a [very curios macro](#map-macro-implementation), let's call it `MAP`:
```cpp
#define MAP(f, ...) /* <some preprocessor magic> */
```
Let's also say we have a macro `F` that takes one argument:
```cpp
#define F(arg) /* <some logic> */
```
A map-macro allows us to map this function-like macro to a variadic list of arguments, for example
```cpp
MAP(F, a, b, c, d)
```
will be evaluate to
```cpp
F(a)
F(b)
F(c)
F(d)
```
Using a similar principle, it is also possible to make a `MAP_LIST` macro which adds commas between the results, which means `MAP_LIST(F, a, b, c, d)` will evaluate to

```cpp
F(a), F(b), F(c), F(d)
```

The implementation of this macro is attached at the [end](#map-macro-implementation), a rather nice explanation of how it works can be found in [this repo](https://github.com/swansontec/map-macro).

Now, one could spend plenty of time figuring out all the preprocessor magic behind the macro, but what I believe is a more important question is: **"What does this give us?"**. From my perspective — an almost perfect tool to implement codegen macros for reflection.

> [!Note]
> Strictly speaking, we will be dealing with a way of providing types with non-intrusive introspection metadata, rather than a proper reflection. A battle-tested implementation of this mechanism is provided [Boost.Describe](https://github.com/boostorg/describe), since the end goals are the same the term "reflection" will be used for conciseness.

> [!Note]
> Ultimately, proper reflection can only be supported at the language level, everything we do before C++26 is trying to bolt-in a language feature using various workarounds.

## `enum` reflection

Let's first establish what do we even want from an `enum` reflection. The main things people usually wish for are:

- Enum <-> string conversion
- Ability to access enumeration elements like an array

This means that minimally we want to achieve an API like this:
```cpp
enum class Side { LEFT, RIGHT };

// ...

static_assert( to_string(Side::LEFT) == "LEFT" );
static_assert( from_string<Side>("LEFT") == Side::LEFT );
static_assert( size<Side> == 2 );
static_assert( values<Side>[0] == Side::LEFT );
static_assert( values<Side>[1] == Side::RIGHT );
```
A map macro makes this almost trivial! All we need is to register enum with a macro:
```cpp
REFLECT_ENUM(Side, LEFT, RIGHT)
```
and have that macro forward our stringified `__VA_ARGS__` into arrays of names and values. While we're at it let's also create array of name-value pairs — this will make iteration more convenient.

Using template specialization as a mechanism for registering types we can do following:
```cpp
// Declare base template that cannot be instantiated
template <class Enum>
struct meta {
    static_assert(always_false_v<Enum>, "Enum has no reflection");
}

// Define macros that we're gonna apply with MAP
#define MAKE_NAME( arg) std::string_view(#arg)
#define MAKE_VALUE(arg) type::arg
#define MAKE_ENTRY(arg) std::pair{ std::string_view(#arg), type::arg }

// Declare specialized template with enum metadata
#define REFLECT_ENUM(enum_name, ...)                                          \
template <>                                                                   \
struct meta<enum_name> {                                                      \
    using type             = enum_name;                                       \
    constexpr auto names   = std::array{ MAP_LIST(MAKE_NAME,  __VA_ARGS__) }; \
    constexpr auto values  = std::array{ MAP_LIST(MAKE_VALUE, __VA_ARGS__) }; \
    constexpr auto entries = std::array{ MAP_LIST(MAKE_VALUE, __VA_ARGS__) }; \
}
```
At this point the trickiest part of the task is basically done — string conversion is just a matter of doing an array lookup for a corresponding type:
```cpp
template<class Enum>
constexpr to_string(Enum e) {
    for (const auto& [name, val] : meta<Enum>::entries) if (e == val) return name;
    throw std::out_of_range("Value is not a part of enum.")
}

template<class Enum>
constexpr from_string(std::string_view str) {
    for (const auto& [name, val] : meta<Enum>::entries) if (str == name) return val;
    throw std::out_of_range("String does not correspond to a value in enum.")
}
```

while the rest can be just trivially wrapped in a public API:

```cpp
template <class Enum> constexpr auto names   = meta<Enum>::names;
template <class Enum> constexpr auto values  = meta<Enum>::values;
template <class Enum> constexpr auto entries = meta<Enum>::entries;
```

> [!Note]
> A more performant option for large enums would be to build a static map in addition to arrays and use it to perform `O(1)` lookup during string conversion, however that falls under "implementation details". Using a map also makes it more difficult to provide functions as `constexpr`.

A "clean" implementation including all of this and some other convenient functions is provided by **utl::enum_reflect** header which can be found [here](../module_enum_reflect.md).

## `struct` reflection

Same as before, let's first identify the basic goals we want to achieve, usually people wish to:

- Treat structures like tuples
- "Iterate" over struct members
- Use the above to implement generic ways to serialize / parse / compare structs.

In short, we want to achieve something like this:

```cpp
struct Config {
    std::string date;
    std::size_t size;
    double      coef;
};

REFLECT_STRUCT(Config, date, size, coef);

// Name reflection
static_assert( size<Config> == 3 );

static_assert( names<Config>[0] == "date" );
static_assert( names<Config>[1] == "size" );
static_assert( names<Config>[2] == "coef" );

// Iterate members with a visitor function
Config cfg = { "2025.03.21", 127, 0.5 };

std::cout << "struct contents = \n";
for_each(cfg, [](const auto& field){ std::cout << field << '\n'; });

// Access struct like a tuple
assert( get<0>(cfg) == "2025.03.21" );

// Treat whole struct as a tuple
auto tuple = field_view(cfg);
assert( std::get<0>(tuple) == "2025.03.21" );
```

> [!Note]
> The are obliviously some other convenient things we would want such as `entry_view()` for name-value pairs, ability to use `for_each()` to define binary operators, predicates and etc. All of this will be provided in the implementation linked at the end.

Same as with `enum`, we can forward stringified `__VA_ARGS__` from register macro and use them to build up a partial specialization with some metadata:

```cpp
// Declare base template that cannot be instantiated
template <class Struct>
struct meta {
    static_assert(_always_false_v<Struct>, "Struct has no reflection.");
};

// Define macros that we're gonna apply with MAP
#define MAKE_NAME( arg) std::string_view(arg)
#define MAKE_VALUE(arg) std::forward<Struct>(value).arg
#define CALL_FUNC( arg) func(std::forward<Struct>(value).arg);

// Declare specialized template with struct metadata
#define REFLECT_STRUCT(struct_name, ...)                                   \
template <>                                                                \
struct meta<struct_name> {                                                 \
    constexpr auto names = std::array{ MAP_LIST(MAKE_NAME, __VA_ARGS__) }; \
                                                                           \
    template <class Struct>                                                \
    constexpr static auto field_view(Struct&& value) noexcept {            \
        return std::forward_as_tuple(MAP_LIST(MAKE_VALUE, __VA_ARGS__));   \
    }                                                                      \
                                                                           \
    template <class Struct, class Func>                                    \
    constexpr static void for_each(Struct&& value, Func&& func) {          \
        MAP(CALL_FUNC, __VA_ARGS__)                                        \
    }                                                                      \
}
```

After this a public API can trivially wrap the calls to metadata:

```cpp
template <class Struct> constexpr auto names = meta<Struct>::names;

template <class Struct>
constexpr auto field_view(Struct&& value) noexcept {
    using struct_type = typename std::decay_t<Struct>;
    return meta<struct_type>::field_view(std::forward<Struct>(value));
}

template <class Struct, class Func>
constexpr void for_each(Struct&& value, Func&& func) {
    using struct_type = typename std::decay_t<Struct>;
    meta<struct_type>::for_each(std::forward<Struct>(value), std::forward<Func>(func));
}

template <std::size_t I, class Struct>
constexpr auto get(Struct&& value) noexcept {
    return std::get<I>(field_view(std::forward<Struct>(value)));
}
```

An there it is, we have all the basic building blocks of a proper reflection!

A "clean" implementation including all of this and some other convenient functions is provided by **utl::struct_reflect** header which can be found [here](../module_struct_reflect.md).

## Some examples

Below are a few small examples showcasing how such reflection can be used.

### Debug printing

Assuming we have a logger than knows how to print tuples, the task of serializing structs for debug purposes becomes almost trivial:

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// ...

// Print struct
using namespace utl;

constexpr Quaternion q = { 0.5, 1.5, 2.5, 3.5 };

log::println("q = ", struct_reflect::entry_view(q));
```

will output:

```
q = < < r, 0.5 >, < i, 1.5 >, < j, 2.5 >, < k, 3.5 > >
```

### Binary operators

A binary version of `for_each()` can be used to implement generic logic for functions & operators:

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

### Generic functions that operate on struct members

```cpp
// Define structs & reflection
struct Vec2 { double x, y; };
struct Vec3 { double x, y, z; };
struct Vec4 { double x, y, z, w; };

UTL_STRUCT_REFLECT(Vec2, x, y);
UTL_STRUCT_REFLECT(Vec3, x, y, z);
UTL_STRUCT_REFLECT(Vec4, x, y, z, w);

// Generic function
template<class T>
constexpr double squared_vector_norm(const T &vec) noexcept {
    double res = 0;
    utl::struct_reflect::for_each(vec, [&](const auto& coord){ res += coord * coord; });
    return res;
}

// Test the function
static_assert( squared_vector_norm(Vec2{1, 2})       ==  5 );
static_assert( squared_vector_norm(Vec3{1, 2, 3})    == 14 );
static_assert( squared_vector_norm(Vec4{1, 2, 3, 4}) == 30 );
```

### Advanced examples

Real-world applications, of course, extend far beyond the toy problems listed above and would be too verbose to fully present here, for example, this exact principle was used to implement reflection for a [utl::json](../module_json.md) parser which knows both how to parse and how to serialize reflected structs with any level of nesting (which means reflected structs can include other reflected structs, nested containers with them and etc.). 

## Why use this over `Boost.PFR` or `Boost.Describe`

- Simpler API
- Supports up to 256 enum elements, `Boost.Describe` is limited to 50
- Everything is `constexpr`, `boost::pfr::for_each_field()` is not
- Single headers (1 for enums, 1 for structs), no dependencies

## Why use `Boost.PFR` or `Boost.Describe` over this

- More features
- Mature libraries
- Works with C++14, `utl` is a C++17 library
- `Boost.PFR` doesn't require registration macros

## Map-macro implementation

```cpp
// Implementation
#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

// Applies function-macro 'F' to all '__VA_ARGS___'
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies function-macro 'F' to all '__VA_ARGS___' and add separator commas
#define MAP_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
```

> [!Note]
> The macro supports up to **364** arguments, in practice this is gonna be limited by compiler evaluation depth which usually caps out at **256**.

## Alternative approaches for `enum` reflection
An alternative approach is taken by [magic_enum](https://github.com/Neargye/magic_enum) and some other libraries, it involves compile-time parsing of strings returned by compiler intrinsics `__PRETTY_FUNCTION__`, `__FUNCSIG__` and C++20 [std::source_location](https://en.cppreference.com/w/cpp/utility/source_location). As a result it is possible to extract enum names without requiring a registration macro, which is quite convenient. This, however, comes at a price of relying on implementation-dependent behavior and introduces some limitations on reflected enums. Compile times also tend suffer through there are some ways to reduce that impact. This approach has its footguns, but is completely viable as showcased by a multitude of libraries implementing it properly.

## Alternative approaches for `struct` reflection

Some other libraries implement reflection using template metadata fields, inheritance and manual declaration of template specializations. Such approach has a benefit of relying more on the language semantics rather than a preprocessor, however it often comes with additional boilerplate and introduces intrusive semantics, making it difficult to use with types provided by 3rd party libraries and other modules.

## Related libraries

| Library                                                                                                | Description                                                                   | Reflection method         |
| ------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------- | ------------------------- |
| [magic_enum](https://github.com/Neargye/magic_enum)                                                    | C++17 feature-rich enum reflection                                            | "Pretty function" parsing |
| [static_enum](https://github.com/KonanM/static_enum)                                                   | C++17 minimalistic enum reflection                                            | "Pretty function" parsing |
| [utl::enum_reflect](../module_enum_reflect.md)                                                         | C++17 minimalistic enum reflection                                            | Map-macro                 |
| [visit_struct](https://github.com/cbeck88/visit_struct)                                                | C++11 minimalistic structure reflection                                       | Map-macro                 |
| [selfaware](https://github.com/jckarter/selfaware)                                                     | C++11 minimalistic structure reflection                                       | Macros & template meta    |
| [utl::struct_reflect](../module_struct_reflect.md)                                                     | C++17 minimalistic structure reflection                                       | Map-macro                 |
| [Glaze](https://github.com/stephenberry/glaze)                                                         | C++23 serialization library that also includes reflection                     | "Pretty function" parsing |
| [reflectcpp](https://github.com/getml/reflect-cpp)                                                     | C++20 serialization library that also includes reflection                     | "Pretty function" parsing |
| [Boost.Hana](https://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html)                      | C++14 metaprogramming library that also includes reflection                   | Macros & template meta    |
| [Boost.Describe](https://github.com/boostorg/describe)                                                 | C++14 reflection library providing very similar API to the one described here | Map-macro                 |
| [Boost.PFR](https://github.com/boostorg/pfr)                                                           | C++14 structure reflection library that doesn't require registration macros   | Custom                    |

| Library                                              | Description                                                                        |
| ---------------------------------------------------- | ---------------------------------------------------------------------------------- |
| [map-macro](https://github.com/swansontec/map-macro) | A clean implementation of the map-macro that inspired this post in the first place |

**Publication date:** 2025.02.23

**Last revision:** 2025.09.03

