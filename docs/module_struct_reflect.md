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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:5,endLineNumber:11,positionColumn:5,positionLineNumber:11,selectionStartColumn:5,selectionStartLineNumber:11,startColumn:5,startLineNumber:11),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+Define+struct+%26+reflection%0Astruct+Quaternion+%7B+double+r,+i,+j,+k%3B+%7D%3B+//+could+be+any+struct+with+a+lot+of+fields%0A%0AUTL_STRUCT_REFLECT(Quaternion,+r,+i,+j,+k)%3B%0A%0Aint+main()+%7B%0A++++//+Test+basic+reflection%0A++++using+namespace+utl%3B%0A++++%0A++++static_assert(+struct_reflect::type_name%3CQuaternion%3E+%3D%3D+%22Quaternion%22+)%3B%0A%0A++++static_assert(+struct_reflect::size%3CQuaternion%3E+%3D%3D+4+)%3B%0A%0A++++static_assert(+struct_reflect::names%3CQuaternion%3E%5B0%5D+%3D%3D+%22r%22+)%3B%0A++++static_assert(+struct_reflect::names%3CQuaternion%3E%5B1%5D+%3D%3D+%22i%22+)%3B%0A++++static_assert(+struct_reflect::names%3CQuaternion%3E%5B2%5D+%3D%3D+%22j%22+)%3B%0A++++static_assert(+struct_reflect::names%3CQuaternion%3E%5B3%5D+%3D%3D+%22k%22+)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGEgMykrgAyeAyYAHI%2BAEaYxCAAHKQADqgKhE4MHt6%2BASlpGQKh4VEssfFJdpgOmUIETMQE2T5%2BXIFVNQJ1DQTFkTFxibb1jc25bcM9faXliQCUtqhexMjsHOb%2BYcjeWADUJv5uCAQEyQogAPQXxEwA7gB0wIQIXtFeSiuyjAT3aCwXABEWIRiHgLKhgOhDKgAG4XOQAFWCFxYTAUBDiF3SRnoAH0tjtMPCkfcEMlkgdsCYNABBak0q67AGYGjhXbo4heBz7MwANl2xBZ9kcAnpHK5BF2AEUvEwMcQGJl9gB2Cy7dBLaL0AWkXZ4XXaXUAawOapMyoBpt2jLQXlo6F2sV2hgAnuyCJzubdns7drRUJLUFRdjRMPaFPT6YjgrihAiAEpyNwI3Hx7AAMWC2GTEBlcriioEurIeoNxrmpsjtLCktRYQgcxVVlpu1b1ouuwRmHRIbwYYd5n532IbphfduEZbbfeYWAu2YbAUySYq12XgItErtPpbd2aAY6MwqmSxGlsvlhYYuwAjvt/ACm7sAKz3XW81%2B7ZUfhL3FWW/zNnSU6tuicp4MguJoh8BAQO6noELigpUMKIAgKG9q4mOmC3BA16Ngc/4Pui6CoQQXjJPQ5pqi%2Bb4fl%2Buo/n%2BuwVgBVY0ruoGOBBUFxDB7okSAwCYAQBxuBolIQOKDiIUK1QEKhw4uph464XMcz3DQxA9nehE8mYxDmGYzFbuxbaceBkEKNBsHEahQkiYc4n%2BNgkkehKMnIXJCmCCOynYap6lKPuA73gRz6/sZrHAe6YHcVZvE2QQAn2aJXASVJCFIShICKX5OF4epmnaQRYWGXghmRYBHH1FxlnWfxdnCal6VudJWVeTlPlKVh%2BVqfcQUCCFunvq2LFVWZNUWTxjSJclTWHGYLXwR52W5T1AUaXgWmSjppVmGY2gVWNO4TbFdUJQ1gnzW4i3Oa5y3tQ43ket1KkFf11SDbt96fhFx3ReZcX1bZV0OW4/hLe5j3yZ1L15RtRU7SVP2GSa%2B2VSdIGTUDF0gylhwQ3dGUrR1a1vX1A0MENYWMRj24A9j50zXBUOyU9oOiU5Ll4d9D4vnTplY2d018cT0ONWDaV3TzyMPiN/2CzFtUi4lD1szD%2BM3RJMuhT9X4C9VwvxczYvqxLomE9z%2BG6w%2BjEK%2Balq0hwCy0JwT68H4HBaKQqCcG41jWOySwrJgPL%2BDwpDyV7zsLEaIDKlw9wJ8qCTh5ICR8gAnFwGhPvonCSB7mi8L7HC8OcGiR8XCxwLAMCICgqAsMkdBxOQlB/C39DxNshjAFwvIaJXNC0PK5wQNExekNEYQNC6nARzPzAjgA8tE2hedwvB/GwggrwwtDz9HpBYG8wBuGItDnFvJ%2BYKiOJrN7%2BCCjUMLdlPR7VOuj%2B8DWYZT7QPA0Qbgjg8FgKeHo8AsAXrwN%2BxBohpEwMye%2BwBAFGGrnwAwwAFAADVxwr2SIwGBMhBAiDEOwKQJD5BKDUFPXQXB9B9xQAHSw%2BggHnEgAsVAyQRQHk4AAWmIgRUwlhrBcGVLsfhK8zAlzgaCLAHCGy2DDHJTILgqZjFaEEKm0wBjxAYakdIvDNF6EMYUBguiyiDAYR0Xh3RRieBaHoWxtQRi9DCP0Kx%2BjJgOJyFo0CjRLGzC4AsBQwdVgSBdm7Iux9S67FUAkXk/DeSSD3AYIwuwB73A0Nk3YEBcCEBIGHEJvAo5aDUqQBAmAmBYHiEo12HBC6kGgU%2BSuntval3LiASuZTnakFrg3JYpx1ztwgJ3VuxAIisDWAkpJKS0l90ye%2BHJ3tMD4CIPIvQ/BSGiHEJQ7Z1CVDqGPvQ0gtwbjJBgVEjg7tSDtJLpwFe65kjrl2EGeJiTkmpN7hkrJKy8keGbhM4pcxSnV1jiASQ75eSZ0kGYLgiTc6ZwTgPfOjTeAtLaVPTpthulV2jhUuOZgfzEvhbyBIT5xH%2BA0AkWFvI0X%2BBiR0zgYKCXXJkXc7FLL8XlIWHA9IzhJBAA%3D%3D%3D) ]

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:7,positionColumn:1,positionLineNumber:7,selectionStartColumn:1,selectionStartLineNumber:7,startColumn:1,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+Define+struct+%26+reflection%0Astruct+Quaternion+%7B+double+r,+i,+j,+k%3B+%7D%3B+//+could+be+any+struct+with+a+lot+of+fields%0A%0AUTL_STRUCT_REFLECT(Quaternion,+r,+i,+j,+k)%3B%0A%0A//+Define+binary+operation+(member-wise+addition)%0Aconstexpr+Quaternion+operator%2B(const+Quaternion%26+lhs,+const+Quaternion+%26rhs)+noexcept+%7B%0A++++Quaternion+res+%3D+lhs%3B%0A++++utl::struct_reflect::for_each(res,+rhs,+%5B%26%5D(auto%26+l,+const+auto%26+r)%7B+l+%2B%3D+r%3B+%7D)%3B%0A++++return+res%3B%0A%7D%0A%0A//+Define+binary+operation+with+predicates+(member-wise+equality)%0Aconstexpr+bool+operator%3D%3D(const+Quaternion%26+lhs,+const+Quaternion+%26rhs)+noexcept+%7B%0A++++bool+res+%3D+true%3B%0A++++utl::struct_reflect::true_for_all(lhs,+rhs,+%5B%26%5D(const+auto%26+l,+const+auto%26+r)%7B+return+l+%3D%3D+r%3B+%7D)%3B%0A++++return+res%3B%0A%7D%0A%0Aint+main()+%7B%0A++++//+Test+operations%0A++++static_assert(+Quaternion%7B1,+2,+3,+4%7D+%2B+Quaternion%7B5,+6,+7,+8%7D+%3D%3D+Quaternion%7B6,+8,+10,+12%7D+)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:10,positionColumn:1,positionLineNumber:10,selectionStartColumn:1,selectionStartLineNumber:10,startColumn:1,startLineNumber:10),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+Define+struct+%26+reflection%0Astruct+Quaternion+%7B+double+r,+i,+j,+k%3B+%7D%3B+//+could+be+any+struct+with+a+lot+of+fields%0A%0AUTL_STRUCT_REFLECT(Quaternion,+r,+i,+j,+k)%3B%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::tuple%3Cstd::string,+int+++%3E+tuple_1%7B+%22lorem%22,+2+%7D%3B%0A++++std::tuple%3Cconst+char*,+double%3E+tuple_2%7B+%22ipsum%22,+3+%7D%3B%0A%0A++++//+Print+tuple%0A++++struct_reflect::tuple_for_each(tuple_1,+%5B%26%5D(auto%26%26+x)%7B+std::cout+%3C%3C+x+%3C%3C+!'%5Cn!'%3B+%7D)%3B%0A%0A++++//+Print+tuple+sum%0A++++struct_reflect::tuple_for_each(tuple_1,+tuple_2,+%5B%26%5D(auto%26%26+x,+auto%26%26+y)%7B+std::cout+%3C%3C+x+%2B+y+%3C%3C+!'%5Cn!'%3B+%7D)%3B%0A%0A++++//+notice+that+tuples+don!'t+have+to+be+homogenous,%0A++++//+what+matters+is+that+binary+function+can+be+called+on+all+corresponding+pairs%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:7,positionColumn:1,positionLineNumber:7,selectionStartColumn:1,selectionStartLineNumber:7,startColumn:1,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+Define+struct+%26+reflection%0Astruct+Quaternion+%7B+double+r,+i,+j,+k%3B+%7D%3B+//+could+be+any+struct+with+a+lot+of+fields%0A%0AUTL_STRUCT_REFLECT(Quaternion,+r,+i,+j,+k)%3B%0A%0Aint+main()+%7B%0A++++//+Print+struct%0A++++using+namespace+utl%3B%0A%0A++++constexpr+Quaternion+q+%3D+%7B+0.5,+1.5,+2.5,+3.5+%7D%3B%0A%0A++++log::println(%22q+%3D+%22,+struct_reflect::entry_view(q))%3B%0A%0A++++//+Note:+there+is+no+tight+coupling+between+the+modules,+%0A++++//+++++++!'utl::log!'+just+knows+how+to+expand+tuples,%0A++++//+++++++other+logger+that+do+this+will+also+work%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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