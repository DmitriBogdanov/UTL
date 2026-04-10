[<img src ="images/badge_language_cpp_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](guide_license.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_docs.svg">](https://dmitribogdanov.github.io/UTL/)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_workflow_windows.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_workflow_ubuntu.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_workflow_macos.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)
[<img src ="images/badge_workflow_freebsd.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/freebsd.yml)

# utl::log

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/log.hpp)

**utl::log** is a lean logging library that tries to make log syntax as simple as possible. It uses type traits to deduce how to serialize various types without depending on its their explicit support, while still providing customization points through formatter specialization. Due to compile-time parametrization & custom formatting the logger achieves significantly lower overhead than standard [`std::ostream`](https://en.cppreference.com/w/cpp/io/basic_ostream.html)-based solutions.

Key features:

- Simple API with no macros
- Serializes [almost every type & container](#serialization-support)
- Automatically adapts to containers with std-like API
- Concise syntax for alignment / color / number formatting
- Sync / async logging with various buffering policies
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

> [!Note]
> While certainly a curious experiment, in hindsight variadic syntax simply doesn't beat [fmt](https://github.com/fmtlib/fmt) for practical use cases. Prefer fmt-based loggers such as [spdlog](https://github.com/gabime/spdlog), [quill](https://github.com/odygrd/quill) or [lwlog](https://github.com/ChristianPanov/lwlog) for production use cases that might require localization.

## Definitions

```cpp
// Logger
template <class... Sinks>
struct logger {
    logger(Sinks&&... sinks);
    
    template <class... Args> void err  (const Args&... args);
    template <class... Args> void warn (const Args&... args);
    template <class... Args> void note (const Args&... args);
    template <class... Args> void info (const Args&... args);
    template <class... Args> void debug(const Args&... args);
    template <class... Args> void trace(const Args&... args);
};

// Sink
template <
    policy::type      type,     = /* inferred from constructor */,
    policy::level     level     = /* defaults based on 'type'  */,
    policy::color     color     = /* defaults based on 'type'  */,
    policy::format    format    = /* defaults based on 'type'  */,
    policy::buffering buffering = /* defaults based on 'type'  */,
    policy::flushing  flushing  = /* defaults based on 'type'  */,
    policy::threading threading = /* defaults based on 'type'  */
> struct sink {
    sink(std::ofstream&&  file); // for file   sinks
    sink(std::string_view name); // for file   sinks
    sink(std::ostream&      os); // for stream sinks
    sink(std::string&      str); // for string sinks
};

// Policies
namespace policy {
    enum class type      { file, stream };
    enum class level     { err, warn, note, info, debug, trace };
    enum class color     { none, ansi };
    enum class format    { date, title, thread, uptime, callsite, level, none, full }; // bitmask
    enum class buffering { none, fixed, timed };
    enum class flushing  { sync, async };
    enum class threading { unsafe, safe };
}

// Default logger
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
constexpr mods::float_format general     (std::size_t precision = 6) noexcept;
constexpr mods::float_format fixed       (std::size_t precision = 3) noexcept;
constexpr mods::float_format scientific  (std::size_t precision = 3) noexcept;
constexpr mods::float_format hex         (std::size_t precision = 3) noexcept;
constexpr mods::int_format   base        (std::size_t base         ) noexcept;
constexpr mods::align_left   align_left  (std::size_t size         ) noexcept;
constexpr mods::align_center align_center(std::size_t size         ) noexcept;
constexpr mods::align_right  align_right (std::size_t size         ) noexcept;
// + all ANSI colors, see methods for the full list

template <class T>
constexpr /*formatted-value*/ operator|(T&& value, /*formatting-mod*/ modifier) noexcept;
```

## Methods

### Logger

> ```cpp
> template <class... Sinks>
> struct logger {
>     logger(Sinks&&... sinks);
>     
>     template <class... Args> void err  (const Args&... args);
>     template <class... Args> void warn (const Args&... args);
>     template <class... Args> void note (const Args&... args);
>     template <class... Args> void info (const Args&... args);
>     template <class... Args> void debug(const Args&... args);
>     template <class... Args> void trace(const Args&... args);
> };
> ```

A **logger** containing one or several sinks.

Functions `err()` / `warn()` / `note()` / `info()` / `debug()` / `trace()` create log entries at corresponding [verbosity levels](#level) with `args...` as a message.

**Note:** The `logger` object can be used [locally](#local-logger) as a regular [RAII](https://en.cppreference.com/w/cpp/language/raii.html) object, or wrapped in a function to work [globally](#global-logger).

### Sink

> ```cpp
> template <
>     policy::type      type,     = /* inferred from constructor */,
>     policy::level     level     = /* defaults based on 'type'  */,
>     policy::color     color     = /* defaults based on 'type'  */,
>     policy::format    format    = /* defaults based on 'type'  */,
>     policy::buffering buffering = /* defaults based on 'type'  */,
>     policy::flushing  flushing  = /* defaults based on 'type'  */,
>     policy::threading threading = /* defaults based on 'type'  */
> > struct sink {
>     sink(std::ofstream&&  file); // for file   sinks
>     sink(std::string_view name); // for file   sinks
>     sink(std::ostream&      os); // for stream sinks
>     sink(std::string&      str); // for string sinks
> };
> ```

Logger **sink** is a wrapper around the file handle ([`std::ofstream`](https://en.cppreference.com/w/cpp/io/basic_ofstream.html)) or stream ([`std::ostream&`](https://en.cppreference.com/w/cpp/io/basic_ostream.html)) that handles writing log messages to them.

`sink` behavior can be customized at compile-time using **policies**. See the [example](#sink-configuration).

By default, the `sink` will infer its `type` based on the constructor argument, while its policies get defaulted to suit the common use case:

| Type                | `type::stream`    | `type::file`       |
| ------------------- | ----------------- | ------------------ |
| Default `level`     | `level::info`     | `level::trace`     |
| Default `color`     | `color::ansi`     | `color::none`      |
| Default `format`    | `format::full`    | `format::full`     |
| Default `buffering` | `buffering::none` | `buffering::fixed` |
| Default `flushing`  | `flushing::sync`  | `flushing::async`  |
| Default `threading` | `threading::safe` | `threading::safe`  |

### Policies

> [!Note]
> All policies reside in a `log::policy` namespace.

#### Type

> ```cpp
> enum class type { file, stream };
> ```

Specifies the **output type** of the sink:

| Value          | Output type                                                  |
| -------------- | ------------------------------------------------------------ |
| `type::file`   | File handle ([`std::ofstream`](https://en.cppreference.com/w/cpp/io/basic_ofstream.html)) |
| `type::stream` | Stream ([`std::ostream&`](https://en.cppreference.com/w/cpp/io/basic_ostream.html)) |

#### Level

> ```cpp
> enum class level { err, warn, note, info, debug, trace };
> ```

Specifies the **verbosity level** of the sink:

| Value          | Verbosity level  |
| -------------- | ---------------- |
| `level::err`   | `err` only       |
| `level::warn`  | `warn` or above  |
| `level::note`  | `note` or above  |
| `level::info`  | `info` or above  |
| `level::debug` | `debug` or above |
| `level::trace` | `trace` or above |

#### Color

> ```cpp
> enum class color { none, ansi };
> ```

Specifies the **color setting** of the sink:

| Value         | Color setting                                                |
| ------------- | ------------------------------------------------------------ |
| `color::none` | Ignore color modifiers                                       |
| `color::ansi` | Use [ANSI escape sequences](https://en.wikipedia.org/wiki/ANSI_escape_code) to format color modifiers |

#### Format

> ```cpp
> enum class format { date, title, thread, uptime, callsite, level, none, full };
> ```

Specifies the **enabled parts** of the sink output:

| Value      | Enabled parts                                       |
| ---------- | --------------------------------------------------- |
| `date`     | Date & time at the top of the log                   |
| `title`    | Column titles at the top of the log                 |
| `thread`   | Thread id column                                    |
| `uptime`   | Uptime in milliseconds column                       |
| `callsite` | Callsite column                                     |
| `level`    | Message level column                                |
| `none`     | Only message is displayed                           |
| `full`     | `date | title | thread | uptime | callsite | level` |

**Note:** This `enum` works like bitmask, for example, value `thread | uptime` will correspond to formatting both columns.

#### Buffering

> ```cpp
> enum class buffering { none, fixed, timed };
> ```

Specifies the **buffering strategy** of the sink output:

| Value              | Buffering strategy                           |
| ------------------ | -------------------------------------------- |
| `buffering::none`  | All output is flushed immediately            |
| `buffering::fixed` | Output is flushed after every 8 KiB          |
| `buffering::timed` | Output is flushed after every 5 milliseconds |

**Note:** Instant buffering tends to be useful during debugging as it ensures no lost messages in case of a crash. Fixed buffering strategy is generally the most reliable in terms of performance. Timed buffering is a hybrid solution that doesn't suffer the full slowdown of instant buffering while still keeping the logs close to the real-time. 

#### Flushing

> ```cpp
> enum class flushing  { sync, async };
> ```

Specifies the **flushing strategy** of the sink output:

| Value             | Flushing strategy                                      |
| ----------------- | ------------------------------------------------------ |
| `flushing::sync`  | Flushing is performed on the same thread               |
| `flushing::async` | Flushing is performed asynchronously on another thread |

**Note:** Async flushing reduces logging latency for the caller, but increases the total amount of work that needs to be done by all threads. It is generally beneficial unless all threads are 100% busy.

#### Threading

> ```cpp
> enum class threading { unsafe, safe };
> ```

Specifies the **thread safety** of the sink output:

| Value               | Thread safety              |
| ------------------- | -------------------------- |
| `threading::unsafe` | Logging is not thread-safe |
| `threading::safe`   | Logging is thread-safe     |

**Note:** Disabling thread safety is generally not advised, but can lead to a performance increase in single-threaded scenarios.

### Default logger

> ```cpp
> template <class... Args> void err  (const Args&... args);
> template <class... Args> void warn (const Args&... args);
> template <class... Args> void note (const Args&... args);
> template <class... Args> void info (const Args&... args);
> template <class... Args> void debug(const Args&... args);
> template <class... Args> void trace(const Args&... args);
> ```

Convenience alias for the `err()` / `warn()` / `note()` / `info()` / `debug()` / `trace()` methods of a pre-defined global logger.

The default logger is lazily initialized upon the first call to these functions, it sinks to [`std::cout`](https://en.cppreference.com/w/cpp/io/cout.html) and `latest.log` file using the default sink policies .

### Printing

> ```cpp
> template <class... Args> void print  (const Args&... args);
> template <class... Args> void println(const Args&... args);
> ```

Prints `args...` to [`std::cout`](https://en.cppreference.com/w/cpp/io/cout.html) using the formatter logic of this library.

This is particularly useful during debugging and general CLI work, as `println()` is both more concise that regular `std::cout` usage and supports a large variety of types that can't be serialized by default. **Formatting modifiers** are also fully supported which allows coloring, alignment and numeric formatting beyond the regular capabilities of stream [`<ios>`](https://en.cppreference.com/w/cpp/header/ios.html).

In addition to this, `println()` is fully thread-safe and locale-independent (unless locale dependency is introduced by the user defining a custom formatter specialization).

> ```cpp
> template <class... Args> std::string stringify(const Args&... args);
> ```

Formats `args...` into an [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string.html) using the formatter logic of this library.

This functions is effectively a universal variadic version of [`std::to_string()`](https://en.cppreference.com/w/cpp/string/basic_string/to_string.html).

**Note:** Due to a heavy compile-time logic utilization, this function is likely to significantly outperform any stringification based on [`std::stringstream`](https://en.cppreference.com/w/cpp/io/basic_stringstream.html). It also heavily outperforms floating-point [`std::to_string()`](https://en.cppreference.com/w/cpp/string/basic_string/to_string.html) and [`sprintf()`](https://en.cppreference.com/w/cpp/io/c/snprintf) due to a more advanced floating-point serialization algorithm based on [`<charconv>`](https://en.cppreference.com/w/cpp/header/charconv.html). Similarly to the `println()`, the output is locale-independent by default.

### Formatting modifiers

> ```cpp
> template <class T>
> constexpr /*formatted-value*/ operator|(T&& value, /*formatting-mod*/ modifier) noexcept;
> ```

Formatting `modifier` can be applied to a `value` by using the `operator|` on its right-hand side.

For example, `x | mod_1 | mod_2` will apply formatting modifiers `mod_1` and `mod_2` to the value `x`.

### Numeric format

> ```cpp
> constexpr mods::float_format general    (std::size_t precision = 6) noexcept;
> constexpr mods::float_format fixed      (std::size_t precision = 3) noexcept;
> constexpr mods::float_format scientific (std::size_t precision = 3) noexcept;
> constexpr mods::float_format hex        (std::size_t precision = 3) noexcept;
> ```

Modifiers that specify the precision and format of a floating point value.

**Note 1:** Only applicable to floating-point values, this is checked at compile-time.

**Note 2:** By default, general format is used with precision chosen according to the shortest representation, see [`std::to_chars()`](https://en.cppreference.com/w/cpp/utility/to_chars.html).

**Note 3:** Standard streams implement similar behavior using [`std::setprecision`](https://en.cppreference.com/w/cpp/io/manip/setprecision.html) in combination with [`std::fixed`](https://en.cppreference.com/w/cpp/io/manip/fixed) / [`std::scientific`](https://en.cppreference.com/w/cpp/io/manip/fixed) / [`std::hexfloat`](https://en.cppreference.com/w/cpp/io/manip/fixed) / [`std::defaultfloat`](https://en.cppreference.com/w/cpp/io/manip/fixed).

> ```cpp
> constexpr mods::int_format base(std::size_t base) noexcept;
> ```

Modifier that specifies the base of an integer value.

**Note 1:** Only applicable to integer values, this is checked at compile-time.

**Note 2:** By default, integers are serialized in base `10`.

**Note 3:** Standard streams implement similar behavior for base `10` / `16` / `8` using [`std::dec`](https://en.cppreference.com/w/cpp/io/manip/hex.html) / [`std::hex`](https://en.cppreference.com/w/cpp/io/manip/hex.html) / [`std::oct`](https://en.cppreference.com/w/cpp/io/manip/hex.html), other arbitrary bases are not supported by standard [`<ios>`](https://en.cppreference.com/w/cpp/header/ios.html).

#### Alignment

> ```cpp
> constexpr mods::align_left   align_left  (std::size_t size) noexcept;
> constexpr mods::align_center align_center(std::size_t size) noexcept;
> constexpr mods::align_right  align_right (std::size_t size) noexcept;
> ```

Modifiers that specify the horizontal alignment of serialized value.

**Note 1:** When serialized value is `size` or more characters long, it is left unchanged.

**Note 2:** Standard streams implement similar behavior using [`std::setw()`](https://en.cppreference.com/w/cpp/io/manip/setw.html) in combination with [`std::left`](https://en.cppreference.com/w/cpp/io/manip/left) / [`std::right`](https://en.cppreference.com/w/cpp/io/manip/left), except there is no manipulator for central alignment.

#### Colors

> ```cpp
> namespace color {
>     constexpr mods::color black;
>     constexpr mods::color red;
>     constexpr mods::color green;
>     constexpr mods::color yellow;
>     constexpr mods::color blue;
>     constexpr mods::color magenta;
>     constexpr mods::color cyan;
>     constexpr mods::color white;
>     constexpr mods::color bright_black;
>     constexpr mods::color bright_red;
>     constexpr mods::color bright_green;
>     constexpr mods::color bright_yellow;
>     constexpr mods::color bright_blue;
>     constexpr mods::color bright_magenta;
>     constexpr mods::color bright_cyan;
>     constexpr mods::color bright_white;
>     constexpr mods::color bold_black;
>     constexpr mods::color bold_red;
>     constexpr mods::color bold_green;
>     constexpr mods::color bold_yellow;
>     constexpr mods::color bold_blue;
>     constexpr mods::color bold_magenta;
>     constexpr mods::color bold_cyan;
>     constexpr mods::color bold_white;
>     constexpr mods::color bold_bright_black;
>     constexpr mods::color bold_bright_red;
>     constexpr mods::color bold_bright_green;
>     constexpr mods::color bold_bright_yellow;
>     constexpr mods::color bold_bright_blue;
>     constexpr mods::color bold_bright_magenta;
>     constexpr mods::color bold_bright_cyan;
>     constexpr mods::color bold_bright_white;
> }
> ```

Modifiers that specify the color & font of the serialized value.

**Note 1:** Coloring mods are implemented using [ANSI escape sequences](https://en.wikipedia.org/wiki/ANSI_escape_code).

**Note 2:** When formatted by a sink with colors disabled, these modifiers will be ignored.

**Note 3:** While ANSI color code support is not entirely ubiquitous, it is provided by most modern terminals.

## Examples

### Basic logging

[ [Run this code](https://godbolt.org/z/KE5jbnE8a) ] [ [Open source file](../examples/module_log/basic_logging.cpp) ]

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

[ [Run this code](https://godbolt.org/z/PT8qhe7Po) ] [ [Open source file](../examples/module_log/logging_objects.cpp) ]

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
auto logger = log::logger{
    log::sink{"log.txt"},
    log::sink{std::cout}
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
    static auto instance = log::logger{
        log::sink{"log.txt"},
        log::sink{std::cout}
    };
    
    return instance;
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
auto logger = log::logger{
    log::sink<
        log::policy::type::file,
        log::policy::level::trace,
        log::policy::color::none,
        log::policy::format::full,
        log::policy::buffering::fixed,
        log::policy::flushing::async,
        log::policy::threading::safe
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
struct vec3 { double x, y, z; };

// Extend formatter to support 'vec3'
template <>
struct log::formatter<vec3> {
    template <class Buffer>
    void operator()(Buffer& buffer, const vec3& vec) {
        formatter<const char*>{}(buffer, "vec3{");
        formatter<     double>{}(buffer, vec.x  );
        formatter<const char*>{}(buffer, ", "   );
        formatter<     double>{}(buffer, vec.y  );
        formatter<const char*>{}(buffer, ", "   );
        formatter<     double>{}(buffer, vec.z  );
        formatter<const char*>{}(buffer, "}"    );
    }
};

// ...

// Test
assert(log::stringify(vec3{1, 2, 3}) == "vec3{1, 2, 3}");
```

### Extending formatter for custom type traits

[ [Run this code]() ] [ [Open source file](../examples/module_log/extending_formatter_for_custom_type_traits.cpp) ]

```cpp
using namespace utl;

// Several custom classes
struct class_1 { std::string to_string() const { return "Class 1"; }; };
struct class_2 { std::string to_string() const { return "Class 2"; }; };
struct class_3 { std::string to_string() const { return "Class 3"; }; };

// Type trait corresponding to those classes
template <class T, class = void>
struct has_to_string : std::false_type {};

template <class T>
struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

// Extend formatter to support anything that provides '.to_string()' member function
template <class T>
struct log::formatter<T, std::enable_if_t<has_to_string<T>::value>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) {
        formatter<std::string>{}(buffer, arg.to_string());
    }
};

// ...

// Test
assert(log::stringify(class_1{}) == "Class 1");
assert(log::stringify(class_2{}) == "Class 2");
assert(log::stringify(class_3{}) == "Class 3");
```

## Serialization support

Serialization of following types is supported out of the box:

- Character types
- Enumerations
- [`std::path`](https://en.cppreference.com/w/cpp/filesystem/path.html) and anything else that provides `.string()`
- Anything convertible to [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view.html)
- Anything convertible to [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string.html)
- Booleans
- Integers
- Floats
- Pointers
- [`std::complex`](https://en.cppreference.com/w/cpp/numeric/complex.html) and anything else that provides `.real()` & `.imag()`
- Array-like types (anything that provides a forward iterator)
- Tuple-like types (anything that supports [`std::get<>()`](https://en.cppreference.com/w/cpp/utility/tuple/get.html) and [`std::tuple_size_v<>`](https://en.cppreference.com/w/cpp/utility/tuple/tuple_size))
- Container adaptors ([`std::queue`](https://en.cppreference.com/w/cpp/container/queue.html), [`std::deque`](https://en.cppreference.com/w/cpp/container/deque.html) and etc.)
- [`<chrono>`](https://en.cppreference.com/w/cpp/header/chrono.html) duration
- Anything printable with [`std::ostream`](https://en.cppreference.com/w/cpp/io/basic_ostream.html)
- Nested containers and types that can be resolved recursively (such as [`std::map`](https://en.cppreference.com/w/cpp/container/map.html), [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map.html) and etc.)

Additional types added by fully or partially specializing the `formatter<>`.

## Useful with modules

- [utl::assertion](module_assertion.md) ‒ can be set up to log assertion failures
- [utl::describe_enum](module_describe_enum.md) ‒ provides C++17 annotation-based reflection for enum serialization
- [utl::describe_struct](module_describe_struct.md) ‒ provides C++17 annotation-based reflection for class serialization
- [utl::reflect_struct](module_reflect_struct.md) ‒ provides C++20 annotation-free reflection for class serialization
- [utl::table](module_table.md) ‒ provides a way to serialize tables
- [utl::time](module_time.md) ‒ provides a way to serialize time and date in various formats
