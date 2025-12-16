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

# utl::sleep

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/sleep.hpp)

**utl::sleep** is a header implementing precise sleep functions.

The main feature is a hybrid sleep function that uses a combination of system sleep, busy-waiting and some statistics to wait with an accuracy close to that of a spinlock, while keeping the thread free most of the time.

The idea is largely based on [this](https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/) excellent blogpost, which introduces the approach and presents a strong case for its robustness on most modern systems.

## Definitions

```cpp
template <class Rep, class Period> void system  (std::chrono::duration<Rep, Period> duration);
template <class Rep, class Period> void spinlock(std::chrono::duration<Rep, Period> duration);
template <class Rep, class Period> void hybrid  (std::chrono::duration<Rep, Period> duration);
```

## Methods

> ```cpp
> template <class Rep, class Period> void system  (std::chrono::duration<Rep, Period> duration);
> template <class Rep, class Period> void spinlock(std::chrono::duration<Rep, Period> duration);
> template <class Rep, class Period> void hybrid  (std::chrono::duration<Rep, Period> duration);
> ```

3 sleep implementations functionally similar to [`std::this_thread::sleep_for()`](https://en.cppreference.com/w/cpp/thread/sleep_for), with different precision / CPU time tradeoffs:

| Sleep        | Precision    | CPU usage | Description                                                  |
| ------------ | ------------ | --------- | ------------------------------------------------------------ |
| `system()`   | Imprecise    | ~0%       | Alias for `std::this_thread::sleep_for()`                    |
| `spinlock()` | Very precise | 100%      | Busy-waiting loop                                            |
| `hybrid()`   | Precise      | ~5%       | Hybrid approach that loops short system sleep, statistically estimates its error on the fly and uses spinlock to finish the last few % |

## Examples

### Comparing sleep precision

[ [Run this code](https://godbolt.org/z/vTfq318oc) ] [ [Open source file](../examples/module_sleep/comparing_sleep_precision.cpp) ]

```cpp
using ms = std::chrono::duration<double, std::milli>;

constexpr int repeats        = 6;
constexpr ms  sleep_duration = ms(16.67);

const auto measure_time = [&](auto sleep_function) {
    for (int i = 0; i < repeats; ++i) {
        const auto start = std::chrono::steady_clock::now();
        sleep_function(sleep_duration);
        const auto end   = std::chrono::steady_clock::now();
        std::cout << ms(end - start).count() << " ms\n";
    }
};

std::cout << "Sleeping for 16.67 ms.\n";

std::cout << "\n--- sleep::system()   ---\n";
measure_time(utl::sleep::system<double, std::milli>);

std::cout << "\n--- sleep::spinlock() ---\n";
measure_time(utl::sleep::spinlock<double, std::milli>);

std::cout << "\n--- sleep::hybrid()   ---\n";
measure_time(utl::sleep::hybrid<double, std::milli>);
```

Output:
```
Sleeping for 16.67 ms.

--- sleep::system()   ---
16.7649 ms
16.7411 ms
16.7406 ms
16.7338 ms
16.7308 ms
16.7338 ms

--- sleep::spinlock() ---
16.6703 ms
16.6702 ms
16.6704 ms
16.6703 ms
16.6703 ms
16.6701 ms

--- sleep::hybrid()   ---
16.6723 ms
16.6723 ms
16.6725 ms
16.6723 ms
16.6721 ms
16.6721 ms
```