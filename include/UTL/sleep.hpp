// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::sleep
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_sleep.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SLEEP)
#ifndef UTLHEADERGUARD_SLEEP
#define UTLHEADERGUARD_SLEEP

#define UTL_SLEEP_VERSION_MAJOR 1
#define UTL_SLEEP_VERSION_MINOR 0
#define UTL_SLEEP_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <chrono>  // steady_clock, duration<>, milli
#include <cmath>   // sqrt()
#include <cstdint> // uint64_t
#include <thread>  // this_thread::sleep_for()

// ____________________ DEVELOPER DOCS ____________________

// Various implementations of sleep, useful for sub-millisecond precision delays.
//
// The interesting part is the hybrid sleep implemented based on the idea of this blogpost:
//    https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
// the blogpost itself contains a mostly suitable implementation, but we can noticeably
// improve it in terms of <chrono> compatibility and general robustness. The main idea
// is quite simple, explanation can be found in the comments.

// ____________________ IMPLEMENTATION ____________________

namespace utl::sleep::impl {

// ======================
// --- Spinlock sleep ---
// ======================

using clock = std::chrono::steady_clock;

template <class Rep, class Period>
void spinlock(std::chrono::duration<Rep, Period> duration) {
    const auto start_timepoint = clock::now();

    volatile std::uint64_t i = 0;

    while (clock::now() - start_timepoint < duration) ++i;

    // volatile 'i' prevents standard-compliant compilers from optimizing away the loop,
    // 'std::uint64_t' is large enough to never overflow and even if it hypothetically would,
    // it still wouldn't be an issue (unlike signed overflow, which would be UB)
}

// ====================
// --- System sleep ---
// ====================

template <class Rep, class Period>
void system(std::chrono::duration<Rep, Period> duration) {
    std::this_thread::sleep_for(duration);
}

// ====================
// --- Hybrid sleep ---
// ====================

// The idea is to loop a short system-sleep, measure actual time elapsed,
// and update our estimate for system-sleep duration error on the fly using
// Welford's algorithm, once remaining duration gets small enough relative to
// expected error we swith to a much more precise spinlock-sleep.
//
// This allows us to be almost as precise as a pure spinlock, but instead of
// taking 100% CPU time we only take a few percent.

template <class Rep, class Period>
void hybrid(std::chrono::duration<Rep, Period> duration) {
    using ms = std::chrono::duration<double, std::milli>; // float duration is easier to work with in our context

    constexpr ms short_sleep_duration = ms(1);

    constexpr double stddev_above_mean = 1;
    // how many standard deviations above the error mean should our estimate be,
    // larger value means a more pessimistic estimate

    thread_local ms            err_estimate = ms(5e-3);     // estimates should be per-thread,
    thread_local ms            err_mean     = err_estimate; // having these 'static' would introduce
    thread_local ms            err_m2       = ms(0);        // a race condition in a concurrent scenario
    thread_local std::uint64_t count        = 1;

    ms remaining_duration = duration;

    while (remaining_duration > err_estimate) {
        // Measure actual system-sleep time
        const auto start = clock::now();
        system(short_sleep_duration);
        const auto end = clock::now();

        const ms observed = end - start;
        remaining_duration -= observed;

        ++count;

        // Update error estimate with Welford's algorithm
        const ms delta = observed - err_mean;
        err_mean += delta / count;
        err_m2 += delta.count() * (observed - err_mean); // intermediate values 'm2' reduce numerical instability
        const ms variance = ms(std::sqrt(err_m2.count() / (count - 1)));

        err_estimate = err_mean + stddev_above_mean * variance;
    }

    spinlock(remaining_duration);
}

} // namespace utl::sleep::impl

// ______________________ PUBLIC API ______________________

namespace utl::sleep {

using impl::system;
using impl::spinlock;
using impl::hybrid;

} // namespace utl::sleep

#endif
#endif // module utl::sleep
