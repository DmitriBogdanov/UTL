// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::time
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_time.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TIME)
#ifndef UTLHEADERGUARD_TIME
#define UTLHEADERGUARD_TIME

#define UTL_TIME_VERSION_MAJOR 1
#define UTL_TIME_VERSION_MINOR 0
#define UTL_TIME_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <chrono>      // steady_clock, system_clock, duration_cast<>(), duration<>, time_point<>
#include <cstddef>     // size_t
#include <ctime>       // strftime, mktime
#include <stdexcept>   // runtime_error
#include <string>      // string, to_string()
#include <type_traits> // common_type_t<>

// ____________________ DEVELOPER DOCS ____________________

// Thin wrapper around <chrono> and <ctime> to make common things easier, initially
// started as 'utl::timer' which was a convenient global-state timer that dealt in doubles.
// After some time and a good read through <chrono> documentation it was deprecated in favor
// of a this 'utl::time' module, the rewrite got rid of any global state and added better type
// safety by properly using <chrono> type system.
//
// The reason we can do things so conveniently is because chrono 'duration' and 'time_moment'
// are capable of wrapping around any arithmetic-like type, including floating-point types which
// are properly supported. This is a bit cumbersome to do "natively" which is why it is rarely
// seen in the wild, but with a few simple wrappers things become quite concise.

// ____________________ IMPLEMENTATION ____________________

namespace utl::time::impl {

// ======================
// --- <chrono> utils ---
// ======================

struct SplitDuration {
    std::chrono::hours        hours;
    std::chrono::minutes      min;
    std::chrono::seconds      sec;
    std::chrono::milliseconds ms;
    std::chrono::microseconds us;
    std::chrono::nanoseconds  ns;

    constexpr static std::size_t size = 6; // number of time units, avoids magic constants everywhere

    using common_rep = std::common_type_t<decltype(hours)::rep, decltype(min)::rep, decltype(sec)::rep,
                                          decltype(ms)::rep, decltype(us)::rep, decltype(ns)::rep>;
    // standard doesn't specify common representation type, usually it's 'std::int64_t'

    std::array<common_rep, SplitDuration::size> count() {
        return {this->hours.count(), this->min.count(), this->sec.count(),
                this->ms.count(),    this->us.count(),  this->ns.count()};
    }
};

template <class Rep, class Period>
[[nodiscard]] constexpr SplitDuration unit_split(std::chrono::duration<Rep, Period> val) {
    // for some reason 'duration_cast<>()' is not 'noexcept'
    const auto hours = std::chrono::duration_cast<std::chrono::hours>(val);
    const auto min   = std::chrono::duration_cast<std::chrono::minutes>(val - hours);
    const auto sec   = std::chrono::duration_cast<std::chrono::seconds>(val - hours - min);
    const auto ms    = std::chrono::duration_cast<std::chrono::milliseconds>(val - hours - min - sec);
    const auto us    = std::chrono::duration_cast<std::chrono::microseconds>(val - hours - min - sec - ms);
    const auto ns    = std::chrono::duration_cast<std::chrono::nanoseconds>(val - hours - min - sec - ms - us);
    return {hours, min, sec, ms, us, ns};
}

template <class Rep, class Period>
[[nodiscard]] std::string to_string(std::chrono::duration<Rep, Period> value, std::size_t relevant_units = 3) {

    // Takes 'unit_count' of the highest relevant units and converts them to string,
    // for example with 'unit_count' equal to '3', we will have:
    //
    // timescale <= hours   =>   show { hours, min, sec }   =>   string "___ hours ___ min ___ sec"
    // timescale <= min     =>   show {   min, sec,  ms }   =>   string "___ min ___ sec ___ ms"
    // timescale <= sec     =>   show {   sec,  ms,  us }   =>   string "___ sec ___ ms ___ us"
    // timescale <= ms      =>   show {    ms,  us,  ns }   =>   string "___ ms ___ us ___ ns"
    // timescale <= us      =>   show {    us,  ns      }   =>   string "___ us ___ ns"
    // timescale <= ns      =>   show {    ns           }   =>   string "___ ns"

    if (relevant_units == 0) return ""; // early escape for a pathological case

    const std::array<SplitDuration::common_rep, SplitDuration::size> counts = unit_split(value).count();
    const std::array<const char*, SplitDuration::size>               names  = {"hours", "min", "sec", "ms", "us", "ns"};

    for (std::size_t unit = 0; unit < counts.size(); ++unit) {
        if (counts[unit]) {
            std::string res;

            const std::size_t last = (unit + relevant_units < counts.size()) ? (unit + relevant_units) : counts.size();
            // don't want to include the whole <algorithm> just for 'std::max()'

            for (std::size_t k = unit; k < last; ++k) {
                res += std::to_string(counts[k]);
                res += ' ';
                res += names[k];
                res += ' ';
            }

            res.resize(res.size() - 1); // remove trailing space at the end

            return res;
        }
    }

    return "0 ns"; // fallback, unlikely to ever be triggered
}

// ===========================
// --- Floating-point time ---
// ===========================

template <class T>
using float_duration = std::chrono::duration<double, typename T::period>;

using ns    = float_duration<std::chrono::nanoseconds>;
using us    = float_duration<std::chrono::microseconds>;
using ms    = float_duration<std::chrono::milliseconds>;
using sec   = float_duration<std::chrono::seconds>;
using min   = float_duration<std::chrono::minutes>;
using hours = float_duration<std::chrono::hours>;

// Note:
// A cool thing about floating-point-represented time is that we don't need 'std::chrono::duration_cast<>()'
// for conversions, float time satisfies 'treat_as_floating_point_v<>' which means implicit conversions between
// duration can happen for any period, in a nutshell instead of this:
//    > std::chrono::duration_cast<time::ms>(std::chrono::nanoseconds(15));
// we can just do this:
//    > time::ms(std::chrono::nanoseconds(15));
// and it's allowed to happen implicitly.

// =================
// --- Stopwatch ---
// =================

template <class Clock = std::chrono::steady_clock>
struct Stopwatch {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;

    Stopwatch() { this->start(); }

    void start() { this->_start = clock::now(); }

    [[nodiscard]] duration elapsed() const { return clock::now() - this->_start; }

    [[nodiscard]] ns    elapsed_ns() const { return this->elapsed(); }
    [[nodiscard]] us    elapsed_us() const { return this->elapsed(); }
    [[nodiscard]] ms    elapsed_ms() const { return this->elapsed(); }
    [[nodiscard]] sec   elapsed_sec() const { return this->elapsed(); }
    [[nodiscard]] min   elapsed_min() const { return this->elapsed(); }
    [[nodiscard]] hours elapsed_hours() const { return this->elapsed(); }
    // <chrono> handles conversion to a floating-point representation when casting duration to the return type

    [[nodiscard]] std::string elapsed_string(std::size_t relevant_units = 3) const {
        return to_string(this->elapsed(), relevant_units);
    }

private:
    time_point _start;
};

// =============
// --- Timer ---
// =============

template <class Clock = std::chrono::steady_clock>
struct Timer {
    using clock      = Clock;
    using time_point = typename clock::time_point;
    using duration   = typename clock::duration;

    Timer() = default;

    template <class Rep, class Period>
    explicit Timer(std::chrono::duration<Rep, Period> length) {
        this->start(length);
    }

    template <class Rep, class Period>
    void start(std::chrono::duration<Rep, Period> length) {
        this->_start  = clock::now();
        this->_length = std::chrono::duration_cast<duration>(length);
    }

    void stop() noexcept { *this = Timer{}; }

    [[nodiscard]] duration elapsed() const { return clock::now() - this->_start; }

    [[nodiscard]] ns    elapsed_ns() const { return this->elapsed(); }
    [[nodiscard]] us    elapsed_us() const { return this->elapsed(); }
    [[nodiscard]] ms    elapsed_ms() const { return this->elapsed(); }
    [[nodiscard]] sec   elapsed_sec() const { return this->elapsed(); }
    [[nodiscard]] min   elapsed_min() const { return this->elapsed(); }
    [[nodiscard]] hours elapsed_hours() const { return this->elapsed(); }
    // <chrono> handles conversion to a floating-point representation when casting duration to the return type

    [[nodiscard]] std::string elapsed_string(std::size_t relevant_units = 3) const {
        return to_string(this->elapsed(), relevant_units);
    }

    [[nodiscard]] bool     finished() const { return this->elapsed() >= this->_length; }
    [[nodiscard]] bool     running() const noexcept { return this->_length != duration{}; }
    [[nodiscard]] duration length() const noexcept { return this->_length; }

private:
    time_point _start{};
    duration   _length{};
};

// ======================
// --- Local datetime ---
// ======================

inline std::tm to_localtime(const std::time_t& time) {
    // There are 3 ways of getting localtime in C-stdlib:
    //    1. 'std::localtime()' - isn't thread-safe and will be marked as "deprecated" by MSVC
    //    2. 'localtime_r()'    - isn't a part of C++, it's a part of C11, in reality provided by POSIX
    //    3. 'localtime_s()'    - isn't a part of C++, it's a part of C23, in reality provided by Windows
    //                            with reversed order of arguments
    // Seemingly there is no portable way of getting thread-safe localtime without being screamed at by at least one
    // compiler, however there is a little known trick that uses a side effect of 'std::mktime()' which normalizes its
    // inputs should they "overflow" the allowed range. Unlike 'localtime', 'std::mktime()' is thread-safe and portable,
    // see https://stackoverflow.com/questions/54246983/c-how-to-fix-add-a-time-offset-the-calculation-is-wrong/54248414

    // Create reference time moment at year 2025
    std::tm reference_tm{};
    reference_tm.tm_isdst = -1;  // negative => let the implementation deal with daylight savings
    reference_tm.tm_year  = 125; // counting starts from 1900

    // Get the 'std::time_t' corresponding to the reference time moment
    const std::time_t reference_time = std::mktime(&reference_tm);
    if (reference_time == -1)
        throw std::runtime_error("time::to_localtime(): time moment can't be represented as 'std::time_t'.");

    // Adjusting reference moment by 'time - reference_time' makes it equal to the current time moment,
    // it is now invalid due to seconds overflowing the allowed range
    reference_tm.tm_sec += static_cast<int>(time - reference_time);
    // 'std::time_t' is an arithmetic type, although not defined, this is almost always an
    // integral value holding the number of seconds since Epoch (see cppreference). This is
    // why we can substract them and add into the seconds.

    // Normalize time moment, it is now valid and corresponds to a current local time
    if (std::mktime(&reference_tm) == -1)
        throw std::runtime_error("time::to_localtime(): time moment can't be represented as 'std::time_t'.");

    return reference_tm;
}

[[nodiscard]] inline std::string datetime_string(const char* format = "%Y-%m-%d %H:%M:%S") {
    const auto now    = std::chrono::system_clock::now();
    const auto c_time = std::chrono::system_clock::to_time_t(now);
    const auto c_tm   = to_localtime(c_time);

    std::array<char, 256> buffer;
    if (std::strftime(buffer.data(), buffer.size(), format, &c_tm) == 0)
        throw std::runtime_error("time::datetime_string(): 'format' does not fit into the buffer.");
    return std::string(buffer.data());

    // Note 1: C++20 provides <chrono> with a native way of getting date, before that we have to use <ctime>
    // Note 2: 'std::chrono::system_clock' is unique - its output can be converted into a C-style 'std::time_t'
    // Note 3: This function is thread-safe, we use a quirky implementation of 'localtime()', see notes above
}

} // namespace utl::time::impl

// ______________________ PUBLIC API ______________________

namespace utl::time {

using impl::SplitDuration;

using impl::unit_split;
using impl::to_string;

using impl::float_duration;

using impl::ns;
using impl::us;
using impl::ms;
using impl::sec;
using impl::min;
using impl::hours;

using impl::Stopwatch;
using impl::Timer;

using impl::to_localtime;
using impl::datetime_string;

} // namespace utl::time

#endif
#endif // module utl::time
