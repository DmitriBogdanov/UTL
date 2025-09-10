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

# utl::strong_type

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/strong_type.hpp)

**utl::strong_type** is a header providing templates for creating **strong typedefs**.

By default, typedefs in `C` and `C++` are **weak**, which means separate typedefs don't count as distinct types:

```cpp
using Offset = std::size_t;
using Size   = std::size_t;

static_assert(std::is_same_v<Offset, Size>); // types are the same
```

**Strong typedefs** can be used to mark types as distinct:

```cpp
using Offset = strong_type::Arithmetic<std::size_t, class OffsetTag>;
using Size   = strong_type::Arithmetic<std::size_t, class   SizeTag>;

static_assert(!std::is_same_v<Offset, Size>); // types are different
```

This is useful for improving type safety. Arithmetic strong types act like thin wrappers around the underlying value and support all of the usual operations, but preserve type and disallow unwanted implicit conversions at no runtime cost.

Strong types are often used in physical modeling together with [`<chrono>`](https://en.cppreference.com/w/cpp/chrono.html)-like ratio conversions to ensure dimensional correctness of the expressions. In a more general case they can protect against mixing up conceptually different values (such as IDs, offsets, sizes and etc.) which would otherwise be implicitly convertible to each other.

In addition, strong types are exceedingly useful for wrapping `C` APIs which tend to use regular integers and type-erased pointers for distinctly different values (whereas `C++` would usually use classes and strongly typed `enum class`). This is particularly common for various system handles, which is why this header also provides `strong_type::Unique<>` that can wrap arbitrary handles into RAII semantics with a custom deleter (see [OpenGL example](#wrapping-opengl-shader-handle)).

## Definitions

```cpp
// Function binding
template <auto function>
struct Bind {
    template <class... Args> constexpr auto operator()(Args&&... args) const;
};

// Strongly typed move-only wrapper around 'T'
template <class T, class Tag, class Deleter = void>
class Unique {
    // Member types
    using   value_type = T;
    using     tag_type = Tag;
    using deleter_type = Deleter;
    
    // Move-only semantics
    constexpr Unique           (const Unique& ) =  delete;
    constexpr Unique& operator=(const Unique& ) =  delete;
    constexpr Unique           (      Unique&&);
    constexpr Unique& operator=(      Unique&&);
    
    // Conversion
    constexpr Unique           (T&& value) noexcept;
    constexpr Unique& operator=(T&& value) noexcept;
    
    // Accessing the underlying value
    constexpr const T& get() const noexcept;
    constexpr       T& get()       noexcept;
};

// Strongly typed arithmetic wrapper around 'T'
template <class T, class Tag>
struct Arithmetic {
    // Member types
    using value_type = T;
    using   tag_type = Tag;
	
    // Conversion
    constexpr Arithmetic           (T value) noexcept;
    constexpr Arithmetic& operator=(T value) noexcept;
    
    // Accessing the underlying value
    constexpr const T& get() const noexcept;
    constexpr       T& get()       noexcept;
    
    // Explicit cast
    template <class To> constexpr explicit operator To() const noexcept;
    
    // + all arithmetic operators supported by 'T'
    // + std::swap() support
};
```

> [!Note]
> Strictly speaking, some of the `noexcept` modifiers listed here are inferred from `std::is_nothrow_move_constructible_v<T>` and other traits. In practice types that can throw during a move are extremely rare for our use case, so it usually holds true.

## Methods

### Function binding

> ```cpp
> template <auto function>
> struct Bind {
>     template <class... Args> constexpr auto operator()(Args&&... args) const;
> };
> ```

Binds `function` to a stateless class so it can be passed as a template parameter.

Useful for passing functions pointers as custom deleters to `std::unique_ptr<>` and `strong_type::Unique<>`.

**Note:** Calling `Bind<function>{}(args...)` is equivalent to calling `function(args...)`.

### Unique

#### Member types

> ```cpp
> using   value_type = T;
> using     tag_type = Tag;
> using deleter_type = Deleter;
> ```

Member types reflecting the template parameters.

`T` can be an instance of any movable type. Default constructor is optional.

`Tag` is an arbitrary class used to discriminate this type from the others.

`Deleter` should either be `void` or a stateless class invocable for `T&&`.

#### Move-only semantics

> ```cpp
> constexpr Unique           (const Unique& ) =  delete;
> constexpr Unique& operator=(const Unique& ) =  delete;
> constexpr Unique           (      Unique&&);
> constexpr Unique& operator=(      Unique&&);
> ```

`Unique<>` is a **move-only** type that behaves similarly to [`std::unique_ptr<>`](https://en.cppreference.com/w/cpp/memory/unique_ptr.html), but can hold an arbitrary internal value.

#### Conversion

> ```cpp
> constexpr Unique           (T&& value) noexcept;
> constexpr Unique& operator=(T&& value) noexcept;
> ```

Constructor / assignment that takes an ownership of the `value`.

**Note:** Previous value (if present) will be destroyed according to the `Deleter` (if present).

#### Accessing the underlying value

> ```cpp
> constexpr const T& get() const noexcept;
> constexpr       T& get()       noexcept;
> ```

Returns a **constant** or **mutable** reference to the managed object.

**Important:** Moving the underlying object out or deleting it can break class invariants. This is impossible to protect against similarly to how  [`std::unique_ptr<>`](https://en.cppreference.com/w/cpp/memory/unique_ptr.html) would cause a double-delete should the user call `delete ptr.get()` manually.

### Arithmetic

#### Member types

> ```cpp
> using value_type = T;
> using   tag_type = Tag;
> ```

Member types reflecting the template parameters.

`T` can be an instance of any [arithmetic](https://en.cppreference.com/w/cpp/types/is_arithmetic.html) type (aka integer or float).

`Tag` is an arbitrary class used to discriminate this type from the others.

#### Conversion

> ```cpp
> constexpr Arithmetic           (T value) noexcept;
> constexpr Arithmetic& operator=(T value) noexcept;
> ```

Constructor / assignment that assigns the underlying `value`.

#### Accessing the underlying value

> ```cpp
> constexpr const T& get() const noexcept;
> constexpr       T& get()       noexcept;
> ```

Returns a **constant** or **mutable** reference to the underlying value.

#### Explicit cast

> ```cpp
> template <class To> constexpr explicit operator To() const noexcept;
> ```

Explicitly casting `Arithmetic<T>` is equivalent to performing the cast on its underlying value.

Implicit casts are intentionally prohibited.

#### Operators

`Arithmetic<T>` supports the same set of binary / unary operators as its underlying `value_type`.

The only exception to this rule is `operator!()` which is intentionally prohibited similarly to implicit casts.

[`std::swap()`](https://en.cppreference.com/w/cpp/algorithm/swap.html) support is also provided.

## Examples

### Wrapping `<cstdio>` file handle

[ [Run this code](https://godbolt.org/z/YGrdsxzjE) ]

```cpp
using namespace utl;

// Create strongly typed wrapper around <cstdio> file handle
// (aka 'FILE*') with move-only semantics and RAII cleanup
using FileHandle = strong_type::Unique<std::FILE*, class FileTag, strong_type::Bind<&std::fclose>>;

FileHandle file = std::fopen("temp.txt", "w");

// upon destruction invokes 'fclose()' on the internal pointer,
// same principle works for most handles produced by 'C' APIs
```

### Wrapping OpenGL shader handle

> [!Note]
> [OpenGL](https://en.wikipedia.org/wiki/OpenGL) is a graphics API written in `C`. It uses `unsigned int` IDs as handles to the objects living in a GPU memory (buffers, shaders, pipelines and etc.). This is a perfect example of an API which greatly benefits from the stronger type safety and automatic cleanup of `strong_type::Unique<>`.

[ [Run this code](https://godbolt.org/z/bfzzE9jzM) ]

```cpp
// Mock of an OpenGL API
using GLuint = unsigned int;
using GLenum = unsigned int;

GLuint glCreateShader ([[maybe_unused]] GLenum shader_type) { return 0; }
void   glCompileShader([[maybe_unused]] GLuint shader_id  ) {           }
void   glDeleteShader ([[maybe_unused]] GLuint shader_id  ) {           }

#define GL_VERTEX_SHADER 1

// ...

using namespace utl;

// Create strongly typed wrapper around OpenGL shader handle 
// (aka 'unsigned int') with move-only semantics and RAII cleanup
using ShaderHandle = strong_type::Unique<GLuint, class ShaderTag, strong_type::Bind<&glDeleteShader>>;

ShaderHandle shader = glCreateShader(GL_VERTEX_SHADER);

// <real OpenGL would also have some boilerplate here>

// Retrieve the underlying value and pass it to a 'C' API
glCompileShader(shader.get());
```

### Strongly typed integer unit

[ [Run this code](https://godbolt.org/z/94sPcxbGK) ]

```cpp
using ByteOffset = utl::strong_type::Arithmetic<int, struct OffsetTag>;

constexpr ByteOffset buffer_start  = 0;
constexpr ByteOffset buffer_stride = 3;

// Perform arithmetics
static_assert(buffer_start + buffer_stride == ByteOffset{3});
static_assert(           2 * buffer_stride == ByteOffset{6});

// Extract value
static_assert(buffer_stride.get() == 3);

// Explicit cast
static_assert(static_cast<int>(buffer_stride) == 3);

// Compile time protection
constexpr int        element_count = 70;
constexpr ByteOffset buffer_end    = buffer_start + element_count * buffer_stride;

// > constexpr ByteOffset buffer_end = buffer_start + element_count;
//   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//   forgot to multiply by stride, will not compile

static_assert(buffer_end == ByteOffset{0 + 3 * 70});
```
