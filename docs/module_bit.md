[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::bit

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/bit.hpp)

**utl::bit** module provides clean implementations for some of the common bit operations. Main highlights are:

- Individual bit manipulation
- Group bit operations
- Works with arbitrary sized integers
- Proper handling of signed types
- Type-safe enum bitflags

> [!Note]
> Significant part of this module gets added into the standard library with **C++20** `<bits>`, all such functions provide similar API to allow for a seamless transition. The only difference is that `std` requires inputs to be unsigned, while `bit` works on both signed & unsigned.

## Definitions

```cpp
// Individual bit operations
template <class T> constexpr bool   get(T value, std::size_t bit) noexcept;
template <class T> constexpr T      set(T value, std::size_t bit) noexcept;
template <class T> constexpr T    clear(T value, std::size_t bit) noexcept;
template <class T> constexpr T     flip(T value, std::size_t bit) noexcept;

// Group bit operations
template <class T> constexpr T lshift(T value, std::size_t shift) noexcept;
template <class T> constexpr T rshift(T value, std::size_t shift) noexcept;
template <class T> constexpr T   rotl(T value, std::size_t shift) noexcept;
template <class T> constexpr T   rotr(T value, std::size_t shift) noexcept;

// Utils
constexpr std::size_t byte_size = CHAR_BIT;

template <class T> constexpr std::size_t size_of;

template <class T> constexpr std::size_t    width(T value) noexcept;
template <class T> constexpr std::size_t popcount(T value) noexcept;

// Enum Bitflags
template<class E>
struct Flags {
    constexpr Flags(                      E       flag) noexcept;
    constexpr Flags(std::initializer_list<E> flag_list) noexcept;
    
    constexpr operator bool() const noexcept;
    constexpr        E  get() const noexcept;
    
    constexpr bool contains(E      flag) const noexcept;
    constexpr bool contains(Flags other) const noexcept;
    
    constexpr Flags&    add(E      flag) noexcept;
    constexpr Flags&    add(Flags other) noexcept;
    constexpr Flags& remove(E      flag) noexcept;
    constexpr Flags& remove(Flags other) noexcept;
    
    // + bit-wise   operators
    // + comparison operators
};
```

## Methods

### Individual bit operations

> ```cpp
> template <class T> constexpr bool get(T value, std::size_t bit) noexcept;
> ```

Returns the state of the `bit` in an integer `value`.

**Note:** This and all consequent functions assume `bit < sizeof(T) * CHAR_BIT`, otherwise behavior is undefined. This precondition is checked in `DEBUG`.

> ```cpp
> template <class T> constexpr T   set(T value, std::size_t bit) noexcept;
> template <class T> constexpr T clear(T value, std::size_t bit) noexcept;
> template <class T> constexpr T  flip(T value, std::size_t bit) noexcept;
> ```

Sets / clears / flips the `bit` in an integer `value` and returns the result.

### Group bit operations

> ```cpp
> template <class T> constexpr T lshift(T value, std::size_t shift) noexcept;
> template <class T> constexpr T rshift(T value, std::size_t shift) noexcept;
> ```

Shifts bits in an integer `value` left / right by a given `shift`, shifted from bits are zero-filled.

Functionally identical to operators `<<` and `>>`, but with a proper handling for negative signed integers. In the standard shifting negative numbers is considered [UB](https://en.cppreference.com/w/cpp/language/ub), in practice every single compiler implements signed bit-shift as `(signed)((unsigned)value << shift)` while implicitly assuming `value > 0` since negatives are UB. This implicit assumption can lead to incorrectly generated code since branches with `value < 0` will be eliminated as "dead code" simply through the virtue of being near a `value << shift` statement. These functions perform the cast explicitly leading to the exact same performance & behavior but safer.

**Note:** This and all consequent functions assume `shift < sizeof(T) * CHAR_BIT`, otherwise behavior is undefined. This precondition is checked in `DEBUG`.

> ```cpp
> template <class T> constexpr T rotl(T value, std::size_t shift) noexcept;
> template <class T> constexpr T rotr(T value, std::size_t shift) noexcept;
> ```

Shifts bits in an integer `value` left / right by a given `shift`, shifted from bits wrap around.

This operation usually compiles down to a single instruction and is often used in computational code.

### Utils

> ```cpp
> constexpr std::size_t byte_size = CHAR_BIT;
> ```

Convenience constant, exists purely to reduce the usage of macros. Evaluates to `8` on most sane platforms.

> ```cpp
> template <class T> constexpr std::size_t size_of;
> ```

Convenience constant. Evaluates to the size of `T` in bits, which equals `sizeof(T) * byte_size`.

> ```cpp
> template <class T> constexpr std::size_t width(T value) noexcept;
> ```

Returns the number of significant bits in an integer.

**Note:** Unsigned integers have `1 + floor(log2(value))` significant bits.

> ```cpp
> template <class T> constexpr std::size_t popcount(T value) noexcept;
> ```

Returns the number of set bits in an integer.

### Enum bitflags

> ```cpp
> constexpr Flags::Flags(                       E      flag) noexcept;
> constexpr Flags::Flags(std::initializer_list<E> flag_list) noexcept;
> ```

Constructs bitflag object from one or several enum values.

> ```cpp
> constexpr operator bool() const noexcept;
> ```

Converts to `false` if underlying bitflag value is `0`, otherwise `true`.

> ```cpp
> constexpr E get() const noexcept;
> ```

Returns the underlying `enum class` value.

> ```cpp
> constexpr bool Flags::contains(E      flag) const noexcept;
> constexpr bool Flags::contains(Flags other) const noexcept;
> ```

Returns whether bitflag object contains a specific flag(s).

> ```cpp
> constexpr Flags&    add(E      flag) noexcept;
> constexpr Flags&    add(Flags other) noexcept;
> constexpr Flags& remove(E      flag) noexcept;
> constexpr Flags& remove(Flags other) noexcept;
> ```

Adds / removes flag(s) from a bitflag object.

Several adds / removes can be chained in a single statement.

> ```cpp
> constexpr Flags operator~() const noexcept;
> 
> constexpr Flags operator|(Flags other) const noexcept;
> constexpr Flags operator&(Flags other) const noexcept;
> 
> constexpr Flags& operator|=(Flags other) noexcept;
> constexpr Flags& operator&=(Flags other) noexcept;
> ```

Bitwise operators used for classic bitflag semantics.

> ```cpp
> constexpr bool operator==(Flags other) noexcept;
> constexpr bool operator!=(Flags other) noexcept;
> constexpr bool operator<=(Flags other) noexcept;
> constexpr bool operator>=(Flags other) noexcept;
> constexpr bool operator< (Flags other) noexcept;
> constexpr bool operator> (Flags other) noexcept;
> ```

Comparison operators, effectively same as comparing the underlying value.

## Examples

### Working with individual bits

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:2,positionColumn:1,positionLineNumber:2,selectionStartColumn:1,selectionStartLineNumber:2,startColumn:1,startLineNumber:2),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++constexpr+std::uint8_t+x+%3D+19%3B+//+19+~+00010011%0A++++//+human-readable+notation+is+big-endian,+which+means+bits+are+indexed+right-to-left%0A%0A++++//+Read+bits%0A++++static_assert(bit::get(x,+0)+%3D%3D+1)%3B%0A++++static_assert(bit::get(x,+1)+%3D%3D+1)%3B%0A++++static_assert(bit::get(x,+2)+%3D%3D+0)%3B%0A++++static_assert(bit::get(x,+3)+%3D%3D+0)%3B%0A++++static_assert(bit::get(x,+4)+%3D%3D+1)%3B%0A++++static_assert(bit::get(x,+5)+%3D%3D+0)%3B%0A++++static_assert(bit::get(x,+6)+%3D%3D+0)%3B%0A++++static_assert(bit::get(x,+7)+%3D%3D+0)%3B%0A%0A++++//+Modify+bits%0A++++static_assert(bit::set(++x,+2)+%3D%3D+23)%3B+//+23+~+00010111%0A++++static_assert(bit::clear(x,+0)+%3D%3D+18)%3B+//+18+~+00010010%0A++++static_assert(bit::flip(+x,+1)+%3D%3D+17)%3B+//+17+~+00010001%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

constexpr std::uint8_t x = 19; // 19 ~ 00010011
// human-readable notation is big-endian, which means bits are indexed right-to-left

// Read bits
static_assert(bit::get(x, 0) == 1);
static_assert(bit::get(x, 1) == 1);
static_assert(bit::get(x, 2) == 0);
static_assert(bit::get(x, 3) == 0);
static_assert(bit::get(x, 4) == 1);
static_assert(bit::get(x, 5) == 0);
static_assert(bit::get(x, 6) == 0);
static_assert(bit::get(x, 7) == 0);

// Modify bits
static_assert(bit::set(  x, 2) == 23); // 23 ~ 00010111
static_assert(bit::clear(x, 0) == 18); // 18 ~ 00010010
static_assert(bit::flip( x, 1) == 17); // 17 ~ 00010001
```

### General usage

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:13,endLineNumber:3,positionColumn:13,positionLineNumber:3,selectionStartColumn:13,selectionStartLineNumber:3,startColumn:13,startLineNumber:3),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++constexpr+std::uint8_t+x+%3D+19%3B+//+19+~+00010011%0A%0A++++//+Group+bit+operations%0A++++static_assert(bit::rotl(++x,+6)+%3D%3D+196)%3B+//+196+~+11000100%0A++++static_assert(bit::rotr(++x,+1)+%3D%3D+137)%3B+//+137+~+10001001%0A++++static_assert(bit::lshift(x,+6)+%3D%3D+192)%3B+//+192+~+11000000%0A++++static_assert(bit::rshift(x,+1)+%3D%3D+++9)%3B+//+++9+~+00001001%0A%0A++++//+Getters%0A++++static_assert(bit::width(x)+%3D%3D+5)%3B+//+00010011+has+5+significant+bits%0A%0A++++static_assert(bit::size_of%3Cstd::uint16_t%3E+%3D%3D+16)%3B%0A++++static_assert(bit::size_of%3Cstd::uint32_t%3E+%3D%3D+32)%3B%0A++++static_assert(bit::size_of%3Cstd::uint64_t%3E+%3D%3D+64)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

constexpr std::uint8_t x = 19; // 19 ~ 00010011

// Group bit operations
static_assert(bit::rotl(  x, 6) == 196); // 196 ~ 11000100
static_assert(bit::rotr(  x, 1) == 137); // 137 ~ 10001001
static_assert(bit::lshift(x, 6) == 192); // 192 ~ 11000000
static_assert(bit::rshift(x, 1) ==   9); //   9 ~ 00001001

// Utils
static_assert(bit::width(x) == 5); // 00010011 has 5 significant bits

static_assert(bit::size_of<std::uint16_t> == 16);
static_assert(bit::size_of<std::uint32_t> == 32);
static_assert(bit::size_of<std::uint64_t> == 64);
```

### Using enum bitflags

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:53,endLineNumber:20,positionColumn:53,positionLineNumber:20,selectionStartColumn:53,selectionStartLineNumber:20,startColumn:53,startLineNumber:20),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Ausing+namespace+utl%3B%0A%0A//+Bitflag-suitable+enum+class%0Aenum+class+IOMode+%7B+IN+%3D+1+%3C%3C+0,+OUT+%3D+1+%3C%3C+1,+APP+%3D+1+%3C%3C+2+%7D%3B%0A%0A//+Function+taking+enum+flags%0Avoid+open_file(bit::Flags%3CIOMode%3E+flags)+%7B%0A++++if+(flags.contains(IOMode::IN)+)+log::println(%22++%3E+File+opened+for+reading%22++)%3B%0A++++if+(flags.contains(IOMode::OUT))+log::println(%22++%3E+File+opened+for+writing%22++)%3B%0A++++if+(flags.contains(IOMode::APP))+log::println(%22++%3E+File+opened+for+appending%22)%3B%0A%7D%0A%0Aint+main()+%7B%0A++++log::println(%22Opening+file+with+OUT:%22)%3B%0A++++open_file(IOMode::OUT)%3B%0A%0A++++log::println(%22Opening+file+with+OUT+%7C+APP:%22)%3B%0A++++open_file(bit::Flags%7BIOMode::OUT,+IOMode::APP%7D)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

// Bitflag-suitable enum class
enum class IOMode { IN = 1 << 0, OUT = 1 << 1, APP = 1 << 2 };

// Function taking enum flags
void open_file(bit::Flags<IOMode> flags) {
    if (flags.contains(IOMode::IN) ) log::println("  > File opened for reading"  );
    if (flags.contains(IOMode::OUT)) log::println("  > File opened for writing"  );
    if (flags.contains(IOMode::APP)) log::println("  > File opened for appending");
}

// ...

log::println("Opening file with OUT:");
open_file(IOMode::OUT);

log::println("Opening file with OUT | APP:");
open_file(bit::Flags{IOMode::OUT, IOMode::APP});
```

Output:
```
Opening file with OUT:
  > File opened for writing
Opening file with OUT | APP:
  > File opened for writing
  > File opened for appending
```

### Additional bitflag examples

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:2,positionColumn:1,positionLineNumber:2,selectionStartColumn:1,selectionStartLineNumber:2,startColumn:1,startLineNumber:2),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Bitflag-suitable+enum+class%0A++++enum+class+IOMode+%7B+IN+%3D+1+%3C%3C+0,+OUT+%3D+1+%3C%3C+1,+APP+%3D+1+%3C%3C+2+%7D%3B%0A%0A++++//+Simple+yet+flexible+API,+same+thing+can+be+accomplished%0A++++//+both+with+classic+bit-wise+semantics+and+with+built-in+methods.%0A++++//+Underneath+it!'s+just+a+strongly+typed+integer+so+there+is+no+performance+impact%0A++++constexpr+auto+flags_1+%3D+bit::Flags%7BIOMode::OUT,+IOMode::APP%7D%3B%0A++++constexpr+auto+flags_2+%3D+bit::Flags(IOMode::OUT)+%7C+bit::Flags(IOMode::APP)%3B%0A++++constexpr+auto+flags_3+%3D+bit::Flags(IOMode::OUT)+%7C+IOMode::APP%3B%0A++++constexpr+auto+flags_4+%3D+bit::Flags(IOMode::OUT).add(IOMode::APP)%3B%0A++++constexpr+auto+flags_5+%3D+bit::Flags%3CIOMode%3E%7B%7D.add(IOMode::OUT).add(IOMode::APP)%3B%0A%0A++++static_assert(flags_1+%3D%3D+flags_2+%26%26+flags_2+%3D%3D+flags_3+%26%26+flags_3+%3D%3D+flags_4+%26%26+flags_4+%3D%3D+flags_5)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:65.37859007832898,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:34.621409921671024,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

// Bitflag-suitable enum class
enum class IOMode { IN = 1 << 0, OUT = 1 << 1, APP = 1 << 2 };

// Simple yet flexible API, same thing can be accomplished
// both with classic bit-wise semantics and with built-in methods.
// Underneath it's just a strongly typed integer so there is no performance impact
constexpr auto flags_1 = bit::Flags{IOMode::OUT, IOMode::APP};
constexpr auto flags_2 = bit::Flags(IOMode::OUT) | bit::Flags(IOMode::APP);
constexpr auto flags_3 = bit::Flags(IOMode::OUT) | IOMode::APP;
constexpr auto flags_4 = bit::Flags(IOMode::OUT).add(IOMode::APP);
constexpr auto flags_5 = bit::Flags<IOMode>{}.add(IOMode::OUT).add(IOMode::APP);

static_assert(flags_1 == flags_2 && flags_2 == flags_3 && flags_3 == flags_4 && flags_4 == flags_5);
```