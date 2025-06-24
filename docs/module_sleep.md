[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::sleep

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/sleep.hpp)

**utl::sleep** implements precise sleep functions.

The main feature is a hybrid sleep function that uses a combination of system sleep, busy-waiting and some statistics to wait with an accuracy close to that of a spinlock, while keeping the thread free most of the time.

The idea is largely based on [this](https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/) excellent blogpost, which introduces the approach and presents a strong case for its robustness on most modern systems.

## Definitions

```cpp
template <class Rep, class Period> void system(  std::chrono::duration<Rep, Period> duration);
template <class Rep, class Period> void spinlock(std::chrono::duration<Rep, Period> duration);
template <class Rep, class Period> void hybrid(  std::chrono::duration<Rep, Period> duration);
```

## Methods

> ```cpp
> template <class Rep, class Period> void system(  std::chrono::duration<Rep, Period> duration);
> template <class Rep, class Period> void spinlock(std::chrono::duration<Rep, Period> duration);
> template <class Rep, class Period> void hybrid(  std::chrono::duration<Rep, Period> duration);
> ```

Three sleep implementations functionally similar to [`std::this_thread::sleep_for()`](https://en.cppreference.com/w/cpp/thread/sleep_for), which cover different ranges of use-cases. Below is a summary table of their properties:

| Sleep        | Precision    | CPU usage | Description                                                  |
| ------------ | ------------ | --------- | ------------------------------------------------------------ |
| `system()`   | Imprecise    | ~0%       | Alias for `std::this_thread::sleep_for()`                    |
| `spinlock()` | Very precise | 100%      | Busy-waiting loop                                            |
| `hybrid()`   | Precise      | ~5%       | Hybrid approach that loops short system sleep, statistically estimates its error on the fly and uses spinlock to finish the last few % |

## Examples

### Comparing sleep precision

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAA7KQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S5yAAqAUuLCY42ilxSRnoAH1trtMAjkQ8EAkEodsCYNABBak0j7BYB7FgKA4bQF7cboEAoBDEASoXnoZZMdqHNzoJYReikLkEHkgEG0Wh4SmHKy0%2BloBgY1QJYh7YIEPbETAJTBitl7G229mcgBsGu1Aj1BpZ1oU9HNuJFt3a9o9EC4DoeDvmzq1tONLKYwQgcwOsU1NLtOvGeyYXiILMtCmWmFxjjYgZMAFYrGXARAszmvZgfVQvAxKgJEyZk/S7Xb%2BIaIDG8IGNBqjey3KbzZaCAoR9ZrHh253ad2V3t0yba6h5V1A9zech%2BYLeRimOgAJ64naoZAAa15DFQdwTkdTq7t9cbzdbDAgH4SvtFdo5hfN801dDdsy3VxbUOTk9z5AUH2PAhLXPS9aGvO8QAfJ9gI2FNQNteC0GzMcJSDaCAFptzqOY/iWQQEzIo4DjMMwPXLNwGHMMwQNXDtAS7GDYkE/D6SEm1iKWE0JXInihG9BImT2Xs9hDMNYg9B5OO4tiXwk%2BVFRImSjjkvSyy4yirK5RTjzPDEWCYm0rMonSeL4m02HRAsizwNgIGzWhj1skAFHslCWAlKU3llQzeWVVVKTwgiYOXIiFX3aTmPHdyLIYFybIbBJjyUhgMNvJiXLcvSxLSzy8x84tMACgggtCkKFFK8qbyi6VYvghK1Q2bBkoMgypNI2SWNyyzrL/XkEDPCIwXQJy9iqvL3Nq196u8s1fP8wLgqKhalpW3qYswOUBroRLhtGrURPpDgFiCjgy14PwOC0UghQ4Nw50sLklhWTBWI2HhSAITQXoWO9Yi4B4EdiAAOCHJBRswHQATi4DQy30ThJE%2BmHfs4XgLg0KGYYWOBYBgRAUFQFglPoMgKAgf5Wf6HZDGAEMNCpmhaBQ4gLggCJSYiYJajPThIel5hiDPAB5CJtAqaHuF4f42EEFWyrl77eCwd5gDcMRaAubXSCwNEcTWH78DNSpYUwa2fswVQKmzR3eGNVpSdVZbZY8LBSYIMEWHl3g3eICJkkwQFMHt4BVSMGm%2BAMYAFAANTwTA7hVi0vsh/hBBEMR2CkGRBEUFR1GN0hdGaAwM9MSxrH0PAIguSAFlQBJ2mtyjuVgjurEsLhNMolWzF4OFohW934AWcpvxcBh3E8Rp/C3noihKbJklSAQRiaRIT/aA%2B%2BhiMZWk1qoJnPvR1/aTo6hvmY78GLoX7GCYX8j5cDXiDVYEhXqcA%2BqQL6P0/p7FUCjB0lEHSSDXG3Zk6kNAPA0HsCAuBCAkHBiA3gWstBzAWAgVC/QEyEw4MTUg0cyxU1gQvcmtgQBUzIS9UgdNGbSQSNmcglAuZ0GiKEVgaxEHINQegvmalQzYM9vgIgK09Dl2EKIcQNcNH1zUKTFupA7i3GKtrSB70SZNz%2BirbMgiTSoCoAgpBKC0G8yMAonBOC8EeBZmIw0mwSHU2NhQ0gd5JChhxpIMwXAkH42xgjEMdCGFMJYaTP6FNOFBPIXDEAZgUYPDydEh0KMyzTw2BoFG2Nwl0I2JYuB7DuEhLevPGBaSGk0wWHHFIzhJBAA%3D%3D) ]

```cpp
using ms = std::chrono::duration<double, std::milli>;

constexpr int repeats        = 6;
constexpr ms  sleep_duration = ms(16.67);

const auto measure_time = [](auto sleep_function) {
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