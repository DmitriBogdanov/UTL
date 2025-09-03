# What separates C++ from being a "perfect" performance-oriented language

**What this is:** A subjective collection of various "performance issues" of C++.

**What constitutes a "performance issue":** Any quirk of the language and its ecosystem that inclines / forces developers to implement things in a suboptimal (in regards to performance) way.

The "issues" are listed in no particular order and were initially compiled as a personal note to summarize a bunch of curios quirks, however after a bit of revision it seems good enough to make for a rather interesting read.

Every "issue" comes with an attached cause and a short explanation.

This does not indent to be a language critique or necessarily make an argument for the sensibility integrating of all listed points, many of the points here are largely presented as "what if".

---

## Performance issues

### No destructive move

**Cause:** Language design.

In C++ any moved-from object is required to stay in some kind of a "valid" moved-from state. This often requires additional logic in the move-constructor and in certain cases can prevent move from being `noexcept` (which can affect performance of standard containers, mainly `std::vector<>`). A good overview of the topic can be found in this [blogpost](https://www.foonathan.net/2017/09/destructive-move/) by Jonathan Müller.

### Implicit copy

**Cause:** Language design.

C++ is a copy-by-default rather than move-by-default language. This makes it easy to accidentally perform a heavyweight copy, which in some cases can hide behind a very innocuous syntax, such as returning a local variable created by a structured binding from a function:

```cpp
SomeHeavyClass f() {
    auto [res, err] = compute();
    // ...
    return res; // this is a copy, no copy elision takes place
}
```

### No `constexpr` priority

**Cause:** Practical concerns.

While `constexpr` functions can be evaluated both at runtime & compile-time, there are no guarantees to which of the options will be chosen unless we manually force the `constexpr` context:

```cpp
constexpr heavy_function() {
    // ...
}

std::cout << heavy_function();
// 'heavy_function()' might be evaluated at runtime unless we force constexpr by assigning it to a constexpr variable first
```

From the idealistic perspective, we always want to pre-compute as much as possible at compile time and having to manually "force" `constexpr` evaluation puts additional burden on the programmer. From a more pragmatic perspective forcing `constexpr` evaluation on everything can have a very significant impact on compilation times due to being significantly slower than a runtime evaluation of the same function.

### Manual struct packing

**Cause:** ABI compatibility, C legacy

In C++ member variables are guaranteed to be stored in the order of their declaration. This can often waste space due to alignment requirements, especially when working with large classes where "performance-optimal" order and "readability-optimal" order might differ:

```cpp
struct Large {
    std::uint32_t a;
    std::uint64_t b;
    std::uint32_t c;
};

struct Small {
    std::uint64_t b;
    std::uint32_t a;
    std::uint32_t c;
};

static_assert(sizeof(Large) == 24);
static_assert(sizeof(Small) == 16);
```

Some languages that prefer static linking leave this ordering to the compiler, in case of C++ the choice was made in favor of compatibility.

### No way to tell when a critical optimization has failed

**Cause:** Compiler QoL

In some scenarios, performance IS correctness. This mainly concerns vectorization, which can fail due to seemingly minor changes and cause a  sudden x2-x4 performance degradation (and degrading, for example, a game from 60 FPS to 15 FPS makes it effectively non-functional).

Unfortunately this is a very complex issue with no easy solution for the problem, most of the time such matters are handled by making a comprehensive benchmark suite with mandated performance degradation tests. An interesting effort was made by the [Unity C# compiler](https://lucasmeijer.com/posts/cpp_unity/) which provides a way to declare data-independence and error should these rules be violated.

### `std::unordered_map` pointer stability

**Cause:** API design

Almost every method of [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map.html) is significantly slower than it could be due to the requirement of [pointer stability](https://en.cppreference.com/w/cpp/container/unordered_map.html#Notes), which dictates a node-based implementation that does not reallocate any elements.

Whether pointer stability is worth the performance cost is frequently debated. An excellent overview of existing map implementations and their performance trade-offs can be found on [Martin Ankerl's website](https://martin.ankerl.com/2022/08/27/hashmap-bench-01/).

As of 2025 it seems that densely stored designs with open addressing & linear probing (such as [`boost::unordered_flat_map`](https://www.boost.org/doc/libs/1_83_0/libs/unordered/doc/html/unordered.html) and [`ankerl::unordered_dense::map`](https://github.com/martinus/unordered_dense)) are a good general go-to.

### `<cmath>` error handling

**Cause:** C legacy

[`<cmath>`](https://en.cppreference.com/w/cpp/header/cmath.html) uses a rather questionable error handling strategy which relies on modifying a global [`errno`](https://en.cppreference.com/w/cpp/error/errno.html) object. In many cases this global access prevents compiler from being more aggressive with optimizations, with prevented vectorization being the biggest issue in terms of impact. For this reason many compilers have an option to disable `errno` reporting (such as `-fno-math-errno` on GCC & clang).

In addition to that, modifying a global variable prevented `<cmath>` functions from being `constexpr` up until C++26, which affects a lot of generic code that could also be compile-time evaluated otherwise.

### `<random>` algorithms

**Cause:** Outdated algorithms

While the core design of [`<random>`](https://en.cppreference.com/w/cpp/header/random.html) is incredibly flexible, its performance suffers from outdated [PRNGs](https://en.wikipedia.org/wiki/Pseudorandom_number_generator) and strict algorithmic requirements for distributions. Switching to a more modern set of algorithms can frequently lead to a **2x-6x** speedup with no loss of statistical quality.

A rather comprehensive overview of this topic can be found in the docs of [utl::random](../module_random.md). 

### `<regex>` performance

**Cause:** ABI stability

Standard library [`<regex>`](https://en.cppreference.com/w/cpp/header/regex.html) is known for its downright horrific performance caused by a suboptimal implementation back in the day. At the moment standard library regex tends to be [**dozens**](https://github.com/mariomka/regex-benchmark) or even [**hundreds**](https://github.com/HFTrader/regex-performance) of times slower than modern regex engines of other languages.

At the API level there is nothing preventing `<regex>` from achieving reasonable performance, however it fell victim to the requirement of ABI stability which set in stone its initial implementation, thus preventing any meaningful improvements in the future.

### No standard 128-bit types

**Cause:** Library support

A lot of bleeding-edge algorithms for [hashing](https://github.com/martinus/unordered_dense/blob/main/include/ankerl/unordered_dense.h#L159-L193), [RNG](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/random.hpp#L119-L179), [serialization](https://github.com/fastfloat/fast_float/blob/main/include/fast_float/bigint.h#L233-L257) and etc. rely on wider type arithmetics which are often natively supported by the architecture (most use cases only need $64 \times 64 \rightarrow 128$ bit arithmetic instructions which are common on modern hardware).

Since every major compiler supports them through extensions (GCC & clang [`__uint128`](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fint128.html), MSVC [`_umul128()`](https://learn.microsoft.com/en-us/cpp/intrinsics/umul128?view=msvc-170) and etc.), this is usually worked around with a bunch of compiler-specific `#ifdef` blocks with an emulated fallback.

Such algorithms would be significantly easier and less error-prone to implement if `uint128_t` & `int128_t` were standardized across compilers. Doing so through [`<cstdint>`](https://en.cppreference.com/w/cpp/header/cstdint.html) however might prove challenging due to the concerns of old code compatibility.

### No bit operations

**Cause:** Library support

**[!] Fixed with:** C++20

Many performant algorithms tend to be written in terms of bit operations present in practically all modern hardware (such as `rotl()`, `popcount()`, `bit_width()`, `countl_zero()`, `bit_cast()` and etc.). Up until C++20 [`<bit>`](https://en.cppreference.com/w/cpp/header/bit.html) there was no portable way to call such instructions, they were usually written in terns of branches & regular shifts and hopefully optimized down to the intended asm by a compiler.

### Floating point parsing & serialization

**Cause:** Library support

**[!] Fixed with:** C++17

Quickly & correctly parsing / serializing floating point numbers is a task of significant complexity, which saw major improvements with the advancement of [Ryu](https://dl.acm.org/ft_gateway.cfm?id=3192369&type=pdf) / [Grisu](https://www.cs.tufts.edu/%7Enr/cs257/archive/florian-loitsch/printf.pdf) / [Schubfach](https://drive.google.com/file/d/1luHhyQF9zKlM8yJ1nebU0OgVYhfC6CBN/view) / [Dragonbox](https://github.com/jk-jeon/dragonbox/blob/master/other_files/Dragonbox.pdf) family of algorithms (speedup of [**several times**](https://github.com/fastfloat/fast_float#how-fast-is-it) with better round-trip guarantees).

Old serialization methods (such as [streams](https://en.cppreference.com/w/cpp/header/ostream.html) and [`std::snprintf()`](https://en.cppreference.com/w/cpp/io/c/snprintf)) are unable to benefit from such advancements due to their legacy requirements. In C++17 [`<charconv>`](https://en.cppreference.com/w/cpp/header/charconv.html) was standardized as a performant low-level way of float serialization which should be more flexible in case of future algorithmic improvements.

### Stream-based formatting

**Cause:** API design

**[!] Fixed with:** C++20, C++23 (partially due to incomplete adoption)

The "classic" way to parse / serialize / format values is based around rather heavyweight polymorphic stream objects that 
conflate formatting, I/O and locale manipulation (which in many cases is largely detrimental).

This approach is **significantly** outperformed by the design of [`fmtlib`](https://github.com/fmtlib/fmt) which was partially standardized in C++20 and C++23.

### No zero-size member optimization

**Cause:** Language design

**[!] Fixed with:** C++20 (partially due to ABI stability concerns)

In C++ all member variables are required to have a valid non-overlapping address. This can pointlessly bloat the object size when working with potentially stateless members (such as allocators, comparators and etc.) whose address is never taken:

```cpp
template <class T, class Allocator = std::allocator<T>, class Comparator = std::less<>>
struct Map {
    Allocator  alloc; // will take space even if stateless
    Comparator comp;  //

    // ...
}
```

This inefficiency used to be worked-around through inheritance hacks and empty base class optimization, however now with C++20 we have a proper attribute [`[[no_unique_address]]`](https://en.cppreference.com/w/cpp/language/attributes/no_unique_address)  to mark those potentially stateless members. Unfortunately, MSVC still [ignores it](https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/#c++20-[[no_unique_address]]) and uses a custom attribute instead.

### No parallel execution guarantees

**Cause:** Library support

While C++17 added [parallel execution modes](https://en.cppreference.com/w/cpp/header/execution.html) to much of the [`<algorithm>`](https://en.cppreference.com/w/cpp/header/algorithm.html), the standard does not mandate that implementations actually have to respect them. For example, GCC will ignore parallel execution modes unless linked against [Intel TBB](https://en.wikipedia.org/wiki/Threading_Building_Blocks), which goes against the intuitive assumption that parallel algorithms should work out of the box.

### Limited reallocation

**Category:** C legacy, complexity

Contiguous arrays such as `std::vector<>` (and other similar classes) should logically be able to grow in-place should the memory allow it, however due to a combination of C `realloc()` design flaws, `std::allocator<>` lack of reallocation mechanism and RAII requirements such ability never made it into the standard (or most of the existing libraries for that matter).

In fact, even further gains could be made if containers had an ability to intrusively ingrate with specific allocators to account for various implementation details, such as, for example [`folly::fbvector`](https://github.com/facebook/folly/blob/main/folly/docs/FBVector.md) which accounts for the [jemalloc](https://github.com/jemalloc/jemalloc) fixed-size quanta. Providing such mechanisms in a general case however proved to be a task of significant complexity.

### Pointer aliasing

**Cause:** Language design

In many scenarios potential [pointer aliasing](https://en.wikipedia.org/wiki/Aliasing_(computing)#Aliased_pointers) can prevent compiler from being more aggressive with optimization, in C we can use [`restrict`](https://en.cppreference.com/w/c/language/restrict.html) to signal a lack of aliasing, in C++ however there is no general solution for the problem.

Many compilers provide extensions such as GCC [`__restrict__`](https://gcc.gnu.org/onlinedocs/gcc/Restricted-Pointers.html), however those qualifiers are only applicable to raw pointers and cannot, for example, specify that two instances of [`std::span<>`](https://en.cppreference.com/w/cpp/container/span.html) are non-aliasing:

```cpp
void vector_sum(std::span<double> res, std::span<double> lhs, std::span<double> rhs) {
    assert(res.size() == lhs.size());
    assert(res.size() == rhs.size());
    
    for (std::size_t i = 0; i < res.size(); ++i) res[i] = lhs[i] + rhs[i];
    // vectorization would be incorrect in a general case due to the potential aliasing
    // when 'res' / lhs' / 'rhs' point to the intersecting chunks of the same array
}
```

For a simple loop like this one many compilers will be able to figure out a special case and use vectorized version when all pointers are proven to be non-aliasing (with a non-vectorized fallback for a general case). In real applications however dependency chains are frequently too complex to be resolved by a compiler, which leads to a significant performance loss relatively to a manually annotated version.

---

## Final notes

The list above was initially made as a personal note to summarize a bunch of curios quirks and as such it does not intend to be a critique of language designers & implementers. Every passing standard makes significant strides in resolving & improving a lot of these points and with C++26 bringing [`std::simd`](https://en.cppreference.com/w/cpp/numeric/simd.html) and reflection we are likely to see some excellent changes in the ecosystem.

**Publication date:** 2025.08.30

**Last revision:** 2025.09.03
