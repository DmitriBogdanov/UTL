[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::time

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/time.hpp)

**utl::time** is a header implementing some simple additions to make [`<chrono>`](https://en.cppreference.com/w/cpp/header/chrono)  less verbose for some common use cases.

Feature summary:

- Split time into hours / minutes / seconds / milliseconds
- Time string formatting
- Floating-point `<chrono>` time units
- `<chrono>`-compatible timers & stopwatches
- C++ wrappers for [`<ctime>`](https://en.cppreference.com/w/cpp/header/ctime) date-time functionality

> [!Note]
> With **C++20** `<chrono>` gets a native way of handling date-time.

## Definitions

```cpp
// Unit split
struct SplitDuration {
    std::chrono::hours        hours;
    std::chrono::minutes      min;
    std::chrono::seconds      sec;
    std::chrono::milliseconds ms;
    std::chrono::microseconds us;
    std::chrono::nanoseconds  ns;
};

template <class Rep, class Period>
SplitDuration unit_split(std::chrono::duration<Rep, Period> value);

template <class Rep, class Period>
std::string to_string(std::chrono::duration<Rep, Period> value, std::size_t relevant_units = 3);

// Floating-point time
template <class T> using float_duration;

using ns    = float_duration<std::chrono::nanoseconds >;
using us    = float_duration<std::chrono::microseconds>;
using ms    = float_duration<std::chrono::milliseconds>;
using sec   = float_duration<std::chrono::seconds     >;
using min   = float_duration<std::chrono::minutes     >;
using hours = float_duration<std::chrono::hours       >;

// Stopwatch
template <class Clock = std::chrono::steady_clock>
struct Stopwatch {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;
    
    Stopwatch();
    void start();
    
    duration    elapsed()        const;
    ns          elapsed_ns()     const;
    us          elapsed_us()     const;
    ms          elapsed_ms()     const;
    sec         elapsed_sec()    const;
    min         elapsed_min()    const;
    hours       elapsed_hours()  const;
    
    std::string elapsed_string(std::size_t relevant_units = 3) const;
};

// Timer
template <class Clock = std::chrono::steady_clock>
struct Timer {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;
    
    Timer();
    
    template <class Rep, class Period>
    explicit Timer(std::chrono::duration<Rep, Period> length);
    
    template <class Rep, class Period>
    void start(std::chrono::duration<Rep, Period> length);
    
    void stop() noexcept;
    
    duration    elapsed()        const;
    ns          elapsed_ns()     const;
    us          elapsed_us()     const;
    ms          elapsed_ms()     const;
    sec         elapsed_sec()    const;
    min         elapsed_min()    const;
    hours       elapsed_hours()  const;
    
    std::string elapsed_string(std::size_t relevant_units = 3) const;
    
    bool     finished() const;
    bool      running() const noexcept;
    duration   length() const noexcept;
};

// Local datetime
std::tm to_localtime(const std::time_t& time);

std::string datetime_string(const char* format = "%Y-%m-%d %H:%M:%S");
```

## Methods

### Unit split

> ```cpp
> struct SplitDuration {
>     std::chrono::hours        hours;
>     std::chrono::minutes      min;
>     std::chrono::seconds      sec;
>     std::chrono::milliseconds ms;
>     std::chrono::microseconds us;
>     std::chrono::nanoseconds  ns;
> };
> ```

POD struct representing duration split into individual units.

> ```cpp
> template <class Rep, class Period>
> SplitDuration unit_split(std::chrono::duration<Rep, Period> value);
>    ```

Splits given duration into distinct units.

For example, `73432` milliseconds will be split into `1` minute, `13` seconds and `432` milliseconds.

> ```cpp
> template <class Rep, class Period>
> std::string to_string(std::chrono::duration<Rep, Period> value, std::size_t relevant_units = 3);
> ```

Converts given duration to a string, showing counts only for the highest `relevant_units`.

See table below for an example:

| Duration             | `relevant_units` | Resulting string           |
| -------------------- | ---------------- | -------------------------- |
| `73432` milliseconds | 4                | `1 min 13 sec 432 ms 0 us` |
| `73432` milliseconds | Default (3)      | `1 min 13 sec 432 ms`      |
| `73432` milliseconds | 2                | `1 min 13 sec`             |
| `73432` milliseconds | 1                | `1 min`                    |
| `73432` milliseconds | 0                | Empty string               |

### Floating-point time

> [!Note]
> While all `std` clocks return time with integer representation, `<chrono>` has native support for all arithmetic types, floating-point-represented time can be used seamlessly with all of the standard functionality.

> ```cpp
> template <class T> using float_duration;
> ```

Returns a floating-point time type corresponding to a given standard time type `T`. Floating-point time uses `double` representation.

> ```cpp
> using ns    = float_duration<std::chrono::nanoseconds >;
> using us    = float_duration<std::chrono::microseconds>;
> using ms    = float_duration<std::chrono::milliseconds>;
> using sec   = float_duration<std::chrono::seconds     >;
> using min   = float_duration<std::chrono::minutes     >;
> using hours = float_duration<std::chrono::hours       >;
> ```

Typedefs for floating-point-represented time units.

### Stopwatch

> ```cpp
> Stopwatch();
> void start();
> ```

Starts the time measurement.

> ```cpp
> duration    elapsed()        const;
> ```

Returns time elapsed since last `start()` (or `Stopwatch` construction) as a `Clock::duration`.

> ```cpp
> ns          elapsed_ns()     const;
> us          elapsed_us()     const;
> ms          elapsed_ms()     const;
> sec         elapsed_sec()    const;
> min         elapsed_min()    const;
> hours       elapsed_hours()  const;
> ```

Returns time elapsed since last `start()` (or `Stopwatch` construction) as a floating-point duration.

> ```cpp
> std::string elapsed_string() const;
> ```

Returns time elapsed since last `start()` (or `Stopwatch` construction) as a formatted `std::string`.

**Note:** See [`time::to_string()`](#unit-split) for an example of output string format.

### Timer

> ```cpp
> Timer();
> ```

Creates timer with a default state. Does not start time measurement.

> ```cpp
> template <class Rep, class Period> explicit Timer(std::chrono::duration<Rep, Period> length);
> template <class Rep, class Period> void     start(std::chrono::duration<Rep, Period> length);
> ```

Starts timer with duration `length`.

>```cpp
> void stop() noexcept;
>```

Stops the timer, returning it to a default state.

> ```cpp
> duration    elapsed()        const;
> ```

Returns time elapsed since last `start()` as a `Clock::duration`.

> ```cpp
> ns          elapsed_ns()     const;
> us          elapsed_us()     const;
> ms          elapsed_ms()     const;
> sec         elapsed_sec()    const;
> min         elapsed_min()    const;
> hours       elapsed_hours()  const;
> ```

Returns time elapsed since last `start()` as a floating-point duration.

> ```cpp
> std::string elapsed_string() const;
> ```

Returns time elapsed since last `start()` as a formatted `std::string`.

**Note:** See [`time::to_string()`](#unit-split) for an example of output string format.

>```cpp
>bool finished() const;
>```

Returns `true` if elapsed time is larger than the timer length, `false` otherwise.

>```cpp
>bool running() const noexcept;
>```

Returns `true` if the timer is currently running, `false` if it's in a default state (either because it wasn't started or because it was explicitly reset with `.stop()`).

>```cpp
>duration length() const noexcept;
>```

Return timer length.

### Local datetime

> ```cpp
> std::tm to_localtime(const std::time_t& time);
> ```

Thread-safe replacement for [`std::localtime()`](https://en.cppreference.com/w/cpp/chrono/c/localtime).

There are 3 ways of getting local time in **C**-stdlib:
1. `std::localtime()` — isn't thread-safe and will be marked as "deprecated" by MSVC
2. `localtime_r()`  — isn't a part of **C++**, it's a part of **C11**, in reality provided by POSIX
3. `localtime_s()`   — isn't a part of **C++**, it's a part of **C23**, in reality provided by Windows with reversed order of arguments

Usually working around this will requires some `#ifdef`'s and non-portable code, however it is possible to exploit some side effects of `std::mktime()` to emulate this function in a thread-safe manner that works on every compiler.

> ```cpp
> std::string datetime_string(const char* format = "%Y-%m-%d %H:%M:%S");
> ```

Returns current date as a string of a given format. Format strings follow [`std::strftime()`](https://en.cppreference.com/w/cpp/chrono/c/strftime) specification.

Thread-safe just like the previous function.

## Examples

### Get elapsed time

[ [Run this code](https://godbolt.org/z/o5nMvdMa9) ]

```cpp
using namespace utl;

const auto some_work = []{ std::this_thread::sleep_for(time::sec(1.7)); };

// Elapsed time as string
time::Stopwatch watch;
some_work();
std::cout << time::to_string(watch.elapsed()) << '\n';

// Elapsed time as double
watch.start();
some_work();
std::cout << watch.elapsed_ms().count()       << '\n';
```

Output:
```
1 sec 700 ms 67 us
1700.48
```

### Accumulate time

[ [Run this code](https://godbolt.org/z/48dsP9qsq) ]

```cpp
using namespace utl;

const auto some_work = [] { std::this_thread::sleep_for(time::sec(0.05)); };

// Accumulate time on 'some_work()' in a loop
time::Stopwatch watch;
time::ms        total{};

for (std::size_t i = 0; i < 20; ++i) {
    watch.start();
    some_work();
    total += watch.elapsed();
}

std::cout << time::to_string(total, 2) << '\n';
```

Output:
```
1 sec 4 ms
```

### Set timers

[ [Run this code](https://godbolt.org/z/7vf36rTh1) ]

```cpp
using namespace utl;

time::Timer   timer;
std::uint64_t count = 0;

timer.start(time::sec(1));
while (!timer.finished()) ++count;
std::cout << "Counted to " << count << " while looping for " << time::to_string(timer.length()) << '\n';
```

Output:
```
Counted to 42286547 while looping for 1 sec 0 ms 0 us
```

### Get local date & time

[ [Run this code](https://godbolt.org/z/1E455Gv7G) ]

```cpp
using namespace utl;

std::cout
    << "Current date:     " << time::datetime_string("%y-%m-%d") << '\n'
    << "Current time:     " << time::datetime_string("%H:%M:%S") << '\n'
    << "Current datetime: " << time::datetime_string()           << '\n';
```

Output:
```
Current date:     25-03-18
Current time:     04:14:13
Current datetime: 2025-03-18 04:14:13
```

## Motivation

Most of the things implemented in this module are quite simple to do using a native `<chrono>` API, however a lot of the time simple and common use cases require a rather hefty amount of boilerplate or tend to be implemented incorrectly by most examples found online.

Below are some of the "motivating examples" showcasing that problem:

### Get elapsed time as `double`

**`std`:**

```cpp
const auto start = std::chrono::steady_clock::now();
std::this_thread::sleep_for(std::chrono::milliseconds(1700)); // wait 1.7 sec
const auto   end = std::chrono::steady_clock::now();
const double  ms = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6;

// this is how it's usually done, however a truly correct way would be to use
// floating-point 'duration_cast()' which ends up being even more verbose:
// const double ms = std::chrono::duration_cast<std::chrono::duration<double, 
//                                              std::chrono::milliseconds::period>>(end - start).count();
```

**`utl::time`:**

```cpp
const time::Stopwatch watch;
std::this_thread::sleep_for(time::sec(1.7));  // wait 1.7 sec
const double ms = watch.elapsed_ms().count();
```

### Convert duration to different units

**`std`:**

```cpp
const auto time_ms = std::chrono::milliseconds(1700);

const auto time_sec = std::chrono::duration_cast<std::chrono::seconds>(time_ms);

// conversion to rougher units is a loss of information (700 ms),
// due to that it requires an explicit 'duration_cast()'

const auto time_ns = std::chrono::nanoseconds(time_ms);

// conversion to finer units happens seamlessly
```

**`utl::time`:**

```cpp
const auto time_ms  = time::ms(1700);

const auto time_sec = time::sec(time_ms);
const auto time_ns  = time::ns (time_ms);

// no loss of precision regardless of units, all conversions happen seamlessly
```

### Get current date & time

**`std`:**

```cpp
const auto now    = std::chrono::system_clock::now();
const auto c_time = std::chrono::system_clock::to_time_t(now);
const auto c_tm   = *std::localtime(&c_time);;

std::array<char, 256> buffer;
std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", &c_tm);

const auto result = std::string(buffer.data());

// not thread-safe, will give a warning in MSVC, we need '#ifdef's to specify 'localtime_r()'
// for POSIX, 'localtime_s()' for Windows and mutex-protected 'std::localtime()' otherwise

// doesn't include error handling, if format string was larger that in this example 'std::strftime()'
// will mess up the formatting and return '0' which usually goes unchecked in examples found online

// in C++20 this gets much easier due to a new <chrono> API
```

**`utl::time`:**

```cpp
const auto result = time::datetime_string();

// thread-safe, compiles everywhere, handles possible format errors
```