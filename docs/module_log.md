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
log::trace("Message 1");
log::info ("Message 2");
log::warn ("Message 3");
```

IMAGE:

```cpp
log::println("Vector ", {1, 2, 3} | log::color::red, " is out of bounds.");
```

IMAGE:

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

```cpp
using namespace utl;

// Log with a default global logger
log::trace("Message 1");
log::info ("Message 2");
log::warn ("Message 3");
```

Output:

TODO:

`latest.log`:

TODO:

### Logging objects

```cpp
using namespace utl;

const auto start = std::chrono::steady_clock::now();

log::info("Starting the solver, x0 = ", std::vector{2e-3, 3e-3, 4e-3}           );
log::warn("Solution diverges, err = " , std::complex<double>{2e14, 3e28}        );
log::err ("Terminated after "         , std::chrono::steady_clock::now() - start);
```

Output:

TODO:

`latest.log`:

TODO:

### Formatting modifiers

> [!Tip]
> The exact same syntax can be used with `println()` / `stringify()`, which is both performant and convenient even outside of logging.

```cpp
using namespace utl;

log::info("Colored:        ", "text" | log::color::red                );
log::info("Left-aligned:   ", "text" | log::align_left(10)            );
log::info("Center-aligned: ", "text" | log::align_center(10)          );
log::info("Right-aligned:  ", "text" | log::align_right(10)           );
log::info("Fixed:          ", 2.3578 | log::fixed(2)                  );
log::info("Scientific:     ", 2.3578 | log::scientific(2)             );
log::info("Hex:            ", 2.3578 | log::hex(2)                    );
log::info("Base-2:         ", 1024   | log::base(2)                   );
log::info("Multiple:       ", 1024   | log::base(2) | log::color::blue);
```

Output:

TODO:

`latest.log`:

TODO:

### Local logger

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

TODO:

`log.txt`:

TODO:

### Global logger

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

TODO:

`log.txt`:

TODO:

### Sink configuration

> [!Tip]
> Most of the time default configuration works well enough: stream sinks are colored with instant buffering, while file sinks are buffered, async and stripped of any color codes.

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
    >{"latest.log"}
};

logger.info("Message 1");
logger.note("Message 2");
logger.warn("Message 3");
```

`latest.log`:


### Extending formatter for custom types

> [!Tip]
> This can also be used to override behavior for types that are already supported, user-defined explicit specialization always gets higher priority.

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
- [utl::table](module_table.md) ‒ provides a way  to serialize tables
- [utl::time](module_time.md) ‒ provides a way to serialize time and date in various formats
