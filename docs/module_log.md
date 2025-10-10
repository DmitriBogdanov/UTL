# utl::loh

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/log.hpp)

**utl::log** is a lean logging library that tries to make syntax as simple as possible. It uses type traits to deduce how to serialize various types without depending on its their support, while still providing customization points through formatter specialization. Due to compile-time parametrization & formatting the logger achieves significantly lower overhead than standard [`std::ostream`]()-based solutions.

Key features:

- Simple API with no macros
- Serializes [almost every type & container](#serialization-support)
- Automatically adapts to containers with std-like API
- Concise syntax for alignment / color / number formatting
- Sync/async logging with various buffering policies
- Convenient `println()` and `stringify()`

Quirks of the library:

- Variadic syntax
- Compile-time parametrization
- Built-in formatting system

**Quick showcase:**

```cpp
log::info("Message 1");
log::warn("Message 2");
log::err ("Message 3");
```

<img src="images/log_basic_logging.png">

```cpp
const auto start = std::chrono::steady_clock::now();

log::println("value   = "  , std::vector{2e-3, 3e-3, 4e-3}           );
log::println("error   = "  , 1.357 | log::scientific(2)              );
log::println("message = "  , "low tolerance" | log::color::bold_red  );
log::println("Finished in ", std::chrono::steady_clock::now() - start);
```

<img src="images/log_showcase_println.png">

## Definitions

```cpp
// Logger
template <class... Sinks>
struct Logger {
    Logger(Sinks&&... sinks);
    
    template <class... Args> void err  (const Args&... args);
    template <class... Args> void warn (const Args&... args);
    template <class... Args> void note (const Args&... args);
    template <class... Args> void info (const Args&... args);
    template <class... Args> void debug(const Args&... args);
    template <class... Args> void trace(const Args&... args);
};

// Sink
template <
    policy::Type      type,     = /* inferred from constructor */,
    policy::Level     level     = /* defaults based on 'type'  */,
    policy::Color     color     = /* defaults based on 'type'  */,
    policy::Format    format    = /* defaults based on 'type'  */,
    policy::Buffering buffering = /* defaults based on 'type'  */,
    policy::Flushing  flushing  = /* defaults based on 'type'  */,
    policy::Threading threading = /* defaults based on 'type'  */,
    class             Callback  = /* empty callback            */
> struct Sink {
    Sink(std::ofstream&&  file); // for file   loggers
    Sink(std::string_view name); // for file   loggers
    Sink(std::ostream&      os); // for stream loggers
    Sink(std::string&      str); // for string loggers
}

// Policies
namespace policy {
    enum class Type      { FILE, STREAM };
    enum class Level     { ERR, WARN, NOTE, INFO, DEBUG, TRACE };
    enum class Color     { NONE, ANSI };
    enum class Format    { DATE, TITLE, THREAD, UPTIME, CALLSITE, LEVEL, NONE, FULL }; // bitmask
    enum class Buffering { NONE, FIXED, TIMED };
    enum class Flushing  { SYNC, ASYNC };
    enum class Threading { UNSAFE, SAFE };
    
    template <auto function> class Callback;
}

// Pre-defined global logger
template <class... Args> void err  (const Args&... args);
template <class... Args> void warn (const Args&... args);
template <class... Args> void note (const Args&... args);
template <class... Args> void info (const Args&... args);
template <class... Args> void debug(const Args&... args);
template <class... Args> void trace(const Args&... args);

// Printing
template <class... Args> void print  (const Args&... args);
template <class... Args> void println(const Args&... args);

template <class... Args> std::string stringify(const Args&... args);

// Formatting modifiers
constexpr mods::FloatFormat general     (std::size_t precision = 6) noexcept;
constexpr mods::FloatFormat fixed       (std::size_t precision = 3) noexcept;
constexpr mods::FloatFormat scientific  (std::size_t precision = 3) noexcept;
constexpr mods::FloatFormat hex         (std::size_t precision = 3) noexcept;
constexpr mods::IntFormat   base        (std::size_t base         ) noexcept;
constexpr mods::AlignLeft   align_left  (std::size_t size         ) noexcept;
constexpr mods::AlignCenter align_center(std::size_t size         ) noexcept;
constexpr mods::AlignRight  align_right (std::size_t size         ) noexcept;
// + all ANSI colors, see methods for the full list

template <class T>
constexpr /*formatted-value*/ operator|(T&& value, /*formatting-mod*/ modifier) noexcept;
```

## Methods

TODO:

## Examples

### Basic logging

[ [Run this code]() ] [ [Open source file](../examples/module_log/basic_logging.cpp) ]

```cpp
using namespace utl;

// Log with a default global logger
log::info("Message 1");
log::warn("Message 2");
log::err ("Message 3");
```

Output:

<img src="images/log_basic_logging.png">

`latest.log`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 03:05:49
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:7    |  INFO | Message 1
| 0      |     0.00 |                     main:8    |  WARN | Message 2
| 0      |     0.00 |                     main:9    |   ERR | Message 3
```

### Logging objects

[ [Run this code]() ] [ [Open source file](../examples/module_log/logging_objects.cpp) ]

```cpp
using namespace utl;

const auto start = std::chrono::steady_clock::now();

log::info("val = "      , std::vector{2e-3, 3e-3, 4e-3}           );
log::warn("err = "      , std::complex<double>{2e14, 3e28}        );
log::err ("Finished in ", std::chrono::steady_clock::now() - start);
```

Output:

<img src="images/log_logging_objects.png">

`latest.log`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 02:58:38
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:10   |  INFO | val = [ 0.002, 0.003, 0.004 ]
| 0      |     0.00 |                     main:11   |  WARN | err = 2e+14 + 3e+28i
| 0      |     0.00 |                     main:12   |   ERR | Finished in 683 us 74 ns
```

### Formatting modifiers

> [!Tip]
> The exact same syntax can be used with `println()` / `stringify()`, which is both performant and convenient even outside of logging.

[ [Run this code]() ] [ [Open source file](../examples/module_log/formatting_modifiers.cpp) ]

```cpp
using namespace utl;

log::note("Colored:        ", "text" | log::color::red                );
log::note("Left-aligned:   ", "text" | log::align_left(10)            );
log::note("Center-aligned: ", "text" | log::align_center(10)          );
log::note("Right-aligned:  ", "text" | log::align_right(10)           );
log::note("Fixed:          ", 2.3578 | log::fixed(2)                  );
log::note("Scientific:     ", 2.3578 | log::scientific(2)             );
log::note("Hex:            ", 2.3578 | log::hex(2)                    );
log::note("Base-2:         ", 1024   | log::base(2)                   );
log::note("Multiple:       ", 1024   | log::base(2) | log::color::blue);
```

Output:

<img src="images/log_formatting_modifiers.png">

`latest.log`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 02:47:01
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:6    |  NOTE | Colored:        text
| 0      |     0.00 |                     main:7    |  NOTE | Left-aligned:   text      
| 0      |     0.00 |                     main:8    |  NOTE | Center-aligned:    text   
| 0      |     0.00 |                     main:9    |  NOTE | Right-aligned:        text
| 0      |     0.00 |                     main:10   |  NOTE | Fixed:          2.36
| 0      |     0.00 |                     main:11   |  NOTE | Scientific:     2.36e+00
| 0      |     0.00 |                     main:12   |  NOTE | Hex:            1.2ep+1
| 0      |     0.00 |                     main:13   |  NOTE | Base-2:         10000000000
| 0      |     0.00 |                     main:14   |  NOTE | Multiple:       10000000000
```

### Local logger

[ [Run this code]() ] [ [Open source file](../examples/module_log/local_logger.cpp) ]

```cpp
using namespace utl;

// Create a local logger
auto logger = log::Logger{
    log::Sink{"log.txt"},
    log::Sink{std::cout}
};

// Use it
logger.info("Message");
```

Output:

<img src="images/log_local_logger.png">

`log.txt`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 03:08:24
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:13   |  INFO | Message
```

### Global logger

[ [Run this code]() ] [ [Open source file](../examples/module_log/global_logger.cpp) ]

```cpp
using namespace utl;

// Create global logger
auto& logger() {
    static auto logger = log::Logger{
        log::Sink{"log.txt"},
        log::Sink{std::cout}
    };
    
    return logger;
}

// ...

// Use it
logger().info("Message");
```

Output:

<img src="images/log_global_logger.png">

`log.txt`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 03:10:57
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:17   |  INFO | Message
```

### Sink configuration

> [!Tip]
> Most of the time default configuration works well enough: stream sinks are colored and flush instantly, while file sinks are buffered, async and stripped of any color codes.

[ [Run this code]() ] [ [Open source file](../examples/module_log/sink_configuration.cpp) ]

```cpp
using namespace utl;

// Verbose async file logger
auto logger = log::Logger{
    log::Sink<
        log::policy::Type::FILE,
        log::policy::Level::TRACE,
        log::policy::Color::NONE,
        log::policy::Format::FULL,
        log::policy::Buffering::FIXED,
        log::policy::Flushing::ASYNC
        log::policy::Threading::SAFE
    >{"latest.log"}
};

logger.info("Message 1");
logger.note("Message 2");
logger.warn("Message 3");
```

`latest.log`:

```
| ------------------------------------------------------------------------------------------
| date -> 2025-10-10 03:12:56
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| thread |   uptime |                      callsite | level | message
| ------ | -------- | ----------------------------- | ----- | ------------------------------
| 0      |     0.00 |                     main:19   |  INFO | Message 1
| 0      |     0.00 |                     main:20   |  NOTE | Message 2
| 0      |     0.00 |                     main:21   |  WARN | Message 3
```

### Extending formatter for custom types

> [!Tip]
> This can also be used to override behavior for types that are already supported, user-defined explicit specialization always gets higher priority.

[ [Run this code]() ] [ [Open source file](../examples/module_log/extending_formatter_for_custom_types.cpp) ]

```cpp
using namespace utl;

// Custom type
struct Vec3 { double x, y, z; };

// Extend formatter to support 'Vec3'
template <>
struct log::Formatter<Vec3> {
    template <class Buffer>
    void operator()(Buffer& buffer, const Vec3& vec) {
        Formatter<const char*>{}(buffer, "Vec3{");
        Formatter<     double>{}(buffer, vec.x  );
        Formatter<const char*>{}(buffer, ", "   );
        Formatter<     double>{}(buffer, vec.y  );
        Formatter<const char*>{}(buffer, ", "   );
        Formatter<     double>{}(buffer, vec.z  );
        Formatter<const char*>{}(buffer, "}"    );
    }
};

// ...

// Test
assert(log::stringify(Vec3{1, 2, 3}) == "Vec3{1, 2, 3}");
```

### Extending formatter for custom type traits

[ [Run this code]() ] [ [Open source file](../examples/module_log/extending_formatter_for_custom_type_traits.cpp) ]

```cpp
using namespace utl;

// Several custom classes
struct Class1 { std::string to_string() const { return "Class 1"; }; };
struct Class2 { std::string to_string() const { return "Class 2"; }; };
struct Class3 { std::string to_string() const { return "Class 3"; }; };

// Type trait corresponding to those classes
template <class T, class = void>
struct has_to_string : std::false_type {};

template <class T>
struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

// Extend formatter to support anything that provides '.to_string()' member function
template <class T>
struct log::Formatter<T, std::enable_if_t<has_to_string<T>::value>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) {
        Formatter<std::string>{}(buffer, arg.to_string());
    }
};

// ...

// Test
assert(log::stringify(Class1{}) == "Class 1");
assert(log::stringify(Class2{}) == "Class 2");
assert(log::stringify(Class2{}) == "Class 3");
```

## Serialization support

Serialization of following types is supported out of the box:

- Character types
- Enumerations
- [`std::path`]() and anything else that provides `.string()`
- Anything convertible to [`std::string_view`]()
- Anything convertible to [`std::string`]()
- Booleans
- Integers
- Floats
- Pointers
- [`std::complex`]() and anything else that provides `.real()` & `.imag()`
- Array-like types (anything that provides a forward iterator)
- Tuple-like types (anything that supports `std::get<>()` and `std::tuple_size_v<>`)
- Container adaptors ([`std::queue`](), [`std::deque`]() and etc.)
- [`<chrono>`]() duration
- Anything printable with [`std::ostream`]()
- Nested containers and types that can be resolved recursively (such as `std::map`, `std::unordered_map` and etc.)

Additional types added by fully or partially specializing the `Formatter<>`.

## Compatibility with other modules

- [utl::assertion](module_assertion.md) ‒ can be set up to log assertion failures
- [utl::enum_reflect](module_enum_reflect.md) ‒ provides an easy way to serialize enums
- [utl::struct_reflect](module_struct_reflect.md) ‒ provides an easy way to serialize classes
- [utl::table](module_table.md) ‒ provides a way to serialize tables
- [utl::time](module_time.md) ‒ provides a way to serialize time and date in various formats
