// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::log
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_log.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_LOG)

#ifndef utl_log_headerguard
#define utl_log_headerguard

#define UTL_LOG_VERSION_MAJOR 2
#define UTL_LOG_VERSION_MINOR 3
#define UTL_LOG_VERSION_PATCH 3

// _______________________ INCLUDES _______________________

#include <array>              // array<>, size_t
#include <atomic>             // atomic<>
#include <cassert>            // assert()
#include <charconv>           // to_chars()
#include <chrono>             // steady_clock
#include <condition_variable> // condition_variable
#include <fstream>            // ofstream, ostream
#include <functional>         // function<>, ref()
#include <iostream>           // cout
#include <iterator>           // ostreambuf_iterator<>
#include <memory>             // unique_ptr<>, make_unique<>()
#include <mutex>              // mutex<>, scoped_lock<>
#include <queue>              // queue<>
#include <sstream>            // ostringstream
#include <string>             // string
#include <string_view>        // string_view
#include <thread>             // thread
#include <tuple>              // tuple<>, get<>, tuple_size_v<>, apply()
#include <type_traits>        // enable_if_t<>, is_enum_v<>, is_integral_v<>, is_unsigned_v<>, underlying_type_t<>, ...
#include <utility>            // forward<>(), integer_sequence<>, make_index_sequence<>

// ____________________ DEVELOPER DOCS ____________________

// A somewhat overengineered logger that uses a lot of compile-time magic and internal macro codegen
// to generate a concise macro-free API that provides features usually associated with macros.
//
// We also want simple customizability on user side while squeezing out as much formatting
// performance as we reasonably can using compile-time logic. Combined with the inherent
// weirdness of variadic formatting this at times leads to some truly horrific constructs.
//
// All things considered, format strings are a much simpler solution from the implementation perspective,
// however properly using them isn't possible without bringing in fmtlib as a dependency. In theory,
// if we wanted to to push for truly top-notch performance the plan would be as follows:
//    1. Bring in fmtlib as a dependency and use format strings as a syntax of choice
//    2. Bring in llfio / glaze or another low latency IO library as a dependency
//    3. Implement async formatting & IO backend:
//    |  A. Loging threads push entries into a MPSC lock-free queue
//    |  B. Serialized types are limited to a specific set which can be encoded
//    |  C. Each queue entry consists of a:
//    |  |  a. Timestamp
//    |  |  b. Pointer to a constexpr metadata struct that contains callsite,
//    |  |     format string and a pointer to a generated template decoder function
//    |  |  c. Binary copy encoding the arguments
//    |  D. Backend thread pops entries from the queue and puts them into an unbounded transit buffer
//    |     ordered by the entry timestamp, this buffer gets periodically flushed, this is necessary
//    |     to sort multi-threaded logs by time without introducing syncronization on the logging
//    |     threads, perfect ordering is not theoretically guaranteed it is good enough in practice
//    |  E. When flushing, backend thread performs:
//    |  |  a. Decoding using the received function pointer
//    |  |  b. Formatting using fmtlib
//    |  |  c. Buffered IO using the manual buffer and the underlying IO library
//    4. Use macro API to generate & format callsite and other metadata at compile-time,
//       doing the same with functions is non-feasible / extremely difficult due to the
//       design of 'std::source_location' which prevent proper compile-time evaluation
//    5. Use a custom <chrono> clock based on RDTSC timestamps for lower overhead
// this, however, would require a project of a whole different scale and integration complexity.

// ____________________ IMPLEMENTATION ____________________

// ===============================================
// --- Optional 'std::source_location' support ---
// ===============================================

// clang-format off

// Detect if we have C++20 or above

#ifdef _MSC_VER
    #define utl_log_cpp_standard _MSVC_LANG
#else
    #define utl_log_cpp_standard __cplusplus
#endif

// For C++20 and above use typedef for 'std::source_location'

#if utl_log_cpp_standard >= 202002L
    #include <source_location>
    namespace utl::log::impl {
        using SourceLocation = std::source_location;
    }
    #define utl_log_has_source_location
#endif

// Below C++20 detect compiler version and intrinsics that allow its emulation.
// Emulation works similarly for all 3 major compilers and is quite simple to implement.
// It provides the entire standard API except '.column()' which is not supported by GCC.

// MSVC: requires at least VS 2019 16.6 Preview 2
#ifndef utl_log_has_source_location
    #ifdef _MSC_VER
        #if _MSC_VER >= 1927
            #define utl_log_use_source_location_builtin
            #define utl_log_has_source_location
        #endif
    #endif
#endif

// GCC, clang: can be detected with '__has_builtin()'
#ifndef utl_log_has_source_location 
    #ifdef __has_builtin
        #if __has_builtin(__builtin_LINE) && __has_builtin(__builtin_FUNCTION) && __has_builtin(__builtin_FILE)
            #define utl_log_use_source_location_builtin
            #define utl_log_has_source_location
        #endif
    #endif
#endif

// clang-format on

#ifdef utl_log_use_source_location_builtin
namespace utl::log::impl {
class SourceLocation {
    int         _line = -1;
    const char* _func = nullptr;
    const char* _file = nullptr;

    constexpr SourceLocation(int line, const char* func, const char* file) noexcept
        : _line(line), _func(func), _file(file) {}

public:
    constexpr SourceLocation()                      = default;
    constexpr SourceLocation(const SourceLocation&) = default;
    constexpr SourceLocation(SourceLocation&&)      = default;

    [[nodiscard]] constexpr static SourceLocation
    current(int line = __builtin_LINE(), const char* func = __builtin_FUNCTION(), const char* file = __builtin_FILE()) {
        return SourceLocation{line, func, file};
    }

    [[nodiscard]] constexpr int         line() const noexcept { return this->_line; }
    [[nodiscard]] constexpr const char* file_name() const noexcept { return this->_file; }
    [[nodiscard]] constexpr const char* function_name() const noexcept { return this->_func; }
};
} // namespace utl::log::impl
#endif

// If all else fails, provide a mock class that returns nothing

#ifndef utl_log_has_source_location
#define utl_log_has_source_location
namespace utl::log::impl {
struct SourceLocation {
    [[nodiscard]] constexpr static SourceLocation current() { return SourceLocation{}; }

    [[nodiscard]] constexpr int         line() const noexcept { return 0; }
    [[nodiscard]] constexpr const char* file_name() const noexcept { return ""; }
    [[nodiscard]] constexpr const char* function_name() const noexcept { return ""; }
};
} // namespace utl::log::impl
#endif

namespace utl::log::impl {

// =================
// --- Utilities ---
// =================

// --- ANSI Colors ---
// -------------------

namespace ansi {

constexpr std::string_view black          = "\033[30m";
constexpr std::string_view red            = "\033[31m";
constexpr std::string_view green          = "\033[32m";
constexpr std::string_view yellow         = "\033[33m";
constexpr std::string_view blue           = "\033[34m";
constexpr std::string_view magenta        = "\033[35m";
constexpr std::string_view cyan           = "\033[36m";
constexpr std::string_view white          = "\033[37m";
constexpr std::string_view bright_black   = "\033[90m"; // also known as "gray"
constexpr std::string_view bright_red     = "\033[91m";
constexpr std::string_view bright_green   = "\033[92m";
constexpr std::string_view bright_yellow  = "\033[93m";
constexpr std::string_view bright_blue    = "\033[94m";
constexpr std::string_view bright_magenta = "\033[95m";
constexpr std::string_view bright_cyan    = "\033[96m";
constexpr std::string_view bright_white   = "\033[97m";

constexpr std::string_view bold_black          = "\033[30;1m";
constexpr std::string_view bold_red            = "\033[31;1m";
constexpr std::string_view bold_green          = "\033[32;1m";
constexpr std::string_view bold_yellow         = "\033[33;1m";
constexpr std::string_view bold_blue           = "\033[34;1m";
constexpr std::string_view bold_magenta        = "\033[35;1m";
constexpr std::string_view bold_cyan           = "\033[36;1m";
constexpr std::string_view bold_white          = "\033[37;1m";
constexpr std::string_view bold_bright_black   = "\033[90;1m";
constexpr std::string_view bold_bright_red     = "\033[91;1m";
constexpr std::string_view bold_bright_green   = "\033[92;1m";
constexpr std::string_view bold_bright_yellow  = "\033[93;1m";
constexpr std::string_view bold_bright_blue    = "\033[94;1m";
constexpr std::string_view bold_bright_magenta = "\033[95;1m";
constexpr std::string_view bold_bright_cyan    = "\033[96;1m";
constexpr std::string_view bold_bright_white   = "\033[97;1m";

constexpr std::string_view reset = "\033[0m";

} // namespace ansi

// --- SFINAE helpers ---
// ----------------------

template <bool Cond>
using require = std::enable_if_t<Cond, bool>;

template <class T>
using require_enum = require<std::is_enum_v<T>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class>
constexpr bool always_false_v = false;

// --- Thread ID ---
// -----------------

// Human readable replacement for 'std::this_thread::get_id()'. Usually this is implemented using
// 'std::unordered_map<std::thread::id, int>' accessed with 'std::this_thread::get_id()' but such
// approach is considerably slower due to a map lookup and requires additional logic to address
// possible thread id reuse, which many implementations seem to neglect. We can use thread-locals
// and an atomic counter to implement lazily initialized thread id count. After a first call the
// overhead of getting thread id should be approx ~ to a branch + array lookup.
[[nodiscard]] inline int get_next_linear_thread_id() {
    static std::atomic<int> counter = 0;
    return counter++;
}

[[nodiscard]] inline int this_thread_linear_id() {
    thread_local int thread_id = get_next_linear_thread_id();
    return thread_id;
}

// --- Local time ---
// ------------------

[[nodiscard]] inline std::tm to_localtime(const std::time_t& time) {
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

// --- Thread-local state ---
// --------------------------

[[nodiscard]] inline std::string& thread_local_temporary_string() {
    thread_local std::string str;
    str.clear();
    return str; // returned string is always empty, no need to clear it at the callsite
}
// In order to compute alignment we have to format things into a temporary string first.
// The naive way would be to allocate a new function-local string every time. We can reduce
// allocations by keeping a single temp string per thread and reusing it every time we need a temporary.

// --- Constants ---
// -----------------

constexpr std::size_t max_chars_float = 30;       // enough to fit 80-bit long double
constexpr std::size_t max_chars_int   = 20;       // enough to fit 64-bit signed integer
constexpr std::size_t buffering_size  = 8 * 1024; // 8 KB, good for most systems
constexpr auto        buffering_time  = std::chrono::milliseconds{5};

// --- <chrono> formatting ---
// ---------------------------

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

using Sec = std::chrono::duration<double, std::chrono::seconds::period>; // convenient duration-to-float conversion

// --- Worker thread ---
// ---------------------

// A single persistent thread that executes detached tasks, effectively a single-thread thread pool
class WorkerThread {
    bool                              terminating;
    std::queue<std::function<void()>> tasks;
    std::mutex                        mutex;
    std::condition_variable           polling_cv;
    std::thread                       thread;

    void thread_main() {
        while (true) {
            std::unique_lock lock(this->mutex);

            this->polling_cv.wait(lock, [this] { return this->terminating || !this->tasks.empty(); });

            while (!this->tasks.empty()) {
                auto task = std::move(this->tasks.front());
                this->tasks.pop();

                lock.unlock();
                task();
                lock.lock();
            }

            if (this->terminating) break;
        }
    }

public:
    WorkerThread() : terminating(false), thread([this] { this->thread_main(); }) {}

    ~WorkerThread() { // stops the worker thread & joins once the tasks are done
        {
            const std::scoped_lock lock(this->mutex);
            this->terminating = true;
        }

        this->polling_cv.notify_one();

        if (this->thread.joinable()) this->thread.join();
    }

    void detached_task(std::function<void()> task) {
        {
            const std::scoped_lock lock(this->mutex);
            this->tasks.emplace(std::move(task));
        }

        this->polling_cv.notify_one();
    }
};

// --- Other ---
// -------------

template <class T>
[[nodiscard]] constexpr T max(T a, T b) noexcept {
    return a < b ? b : a;
} // saves us from including the whole <algorithm> for a one-liner

template <class Tp, class Func> // Func = void(Element&&)
constexpr void tuple_for_each(Tp&& tuple, Func&& func) {
    std::apply([&func](auto&&... args) { (func(std::forward<decltype(args)>(args)), ...); }, std::forward<Tp>(tuple));
}

template <class T, T... Idxs, class Func> // Func = void(std::integral_constant<T, Index>)
constexpr void for_sequence(std::integer_sequence<T, Idxs...>, Func&& func) {
    (func(std::integral_constant<std::size_t, Idxs>{}), ...);
} // effectively a 'constexpr for' where 'constexpr parameters' are passed as integral constants

template <class E, require_enum<E> = true>
[[nodiscard]] constexpr auto to_underlying(E value) noexcept {
    return static_cast<std::underlying_type_t<E>>(value); // in C++23 gets replaced by 'std::to_underlying()'
}

template <class E, require_enum<E> = true>
[[nodiscard]] constexpr bool to_bool(E value) noexcept {
    return static_cast<bool>(to_underlying(value));
}

// "Unwrapper" for container adaptors such as 'std::queue', 'std::priority_queue', 'std::stack'
template <class Adaptor>
const auto& underlying_container_cref(const Adaptor& adaptor) {

    struct Hack : private Adaptor {
        static const typename Adaptor::container_type& get_container(const Adaptor& adp) {
            return adp.*&Hack::c;
            // An extremely hacky yet standard-compliant way of accessing protected member
            // of a class without actually creating any instances of the derived class.
            //
            // This expression consists of 2 operators: '.*' and '&'.
            // '.*' takes an object on the left side, and a member pointer on the right side,
            // and resolves the pointed-to member of the given object, this means
            // 'object.*&Class::member' is essentially equivalent to 'object.member',
            // except it reveals a "loophole" in protection semantics that allows us to
            // interpret base class object as if it was derived class.
            //
            // Note that doing seemingly much more logical 'static_cast<Derived&>(object).member'
            // is technically UB, even through most compilers will do the reasonable thing.
        }
    };

    return Hack::get_container(adaptor);
}

// ===============
// --- Styling ---
// ===============

// --- Modifiers ---
// -----------------

// clang-format off
namespace mods {
struct FloatFormat   { std::chars_format format; int precision; };
struct IntegerFormat { int               base;                  };
struct AlignLeft     { std::size_t       size;                  };
struct AlignCenter   { std::size_t       size;                  };
struct AlignRight    { std::size_t       size;                  };
struct Color         { std::string_view  code;                  };
} // namespace mods
// clang-format on

// --- Wrapped values ---
// ----------------------

// Note:
// Creating a 'wrapped value' class that doesn't leave dangling references when used with temporaries
// requires careful application of perfect forwarding, if we were to make a naive struct like this:
//    > template <class T>
//    > struct Wrapper { const T& val };
// Then creating such struct from a temporary and then passing it around further would lead to a dangling reference,
// we want 'Wrapper' to "take ownership" of the value when it gets constructed from a temporary, but save only a
// reference when it gets constructed from an l-value (since copying it would be wasteful), this can be achieved
// through appropriate CTAD with perfect forwarding:
//    > template <class T>
//    > struct Wrapper {
//    >     T val;
//    >     Wrapper(T&& val) : val(std::forward<T>(val)) {}
//    > };
//    >
//    > template <class T>
//    > Wrapper(T&& val) -> Wrapper<T>;
// 'Wrapper{const std::vector<int>&}' => 'T' will be 'const std::vector<int>&'
// 'Wrapper{      std::vector<int>&}' => 'T' will be '      std::vector<int>&'
// 'Wrapper{      std::vector<int> }' => 'T' will be '      std::vector<int> '

#define utl_log_define_style_wrapper(wrapper_, style_)                                                                 \
    template <class T>                                                                                                 \
    struct wrapper_ {                                                                                                  \
        T      value;                                                                                                  \
        style_ mod;                                                                                                    \
                                                                                                                       \
        wrapper_(T&& value, style_ mod) : value(std::forward<T>(value)), mod(mod) {}                                   \
    };                                                                                                                 \
                                                                                                                       \
    template <class T>                                                                                                 \
    wrapper_(T&&, style_)->wrapper_<T>

// clang-format off
utl_log_define_style_wrapper(FormattedFloat  , mods::FloatFormat  );
utl_log_define_style_wrapper(FormattedInteger, mods::IntegerFormat);
utl_log_define_style_wrapper(AlignedLeft     , mods::AlignLeft    );
utl_log_define_style_wrapper(AlignedCenter   , mods::AlignCenter  );
utl_log_define_style_wrapper(AlignedRight    , mods::AlignRight   );
utl_log_define_style_wrapper(Colored         , mods::Color        );
// clang-format on

#undef utl_log_define_style_wrapper

// --- Style merging ---
// ---------------------

namespace mods { // necessary for unqualified name lookup to discover operators with 'mods::' types

#define utl_log_define_style_merging(wrapper_, style_, require_)                                                       \
    template <class T, require_ = true>                                                                                \
    [[nodiscard]] constexpr wrapper_<T> operator|(T&& value, style_ mod) noexcept {                                    \
        return {std::forward<T>(value), mod};                                                                          \
    }                                                                                                                  \
    static_assert(true)

// clang-format off
utl_log_define_style_merging(FormattedFloat  , mods::FloatFormat  , require_float   <std::decay_t<T>>);
utl_log_define_style_merging(FormattedInteger, mods::IntegerFormat, require_integral<std::decay_t<T>>);
utl_log_define_style_merging(AlignedLeft     , mods::AlignLeft    , bool                             );
utl_log_define_style_merging(AlignedCenter   , mods::AlignCenter  , bool                             );
utl_log_define_style_merging(AlignedRight    , mods::AlignRight   , bool                             );
utl_log_define_style_merging(Colored         , mods::Color        , bool                             );
// clang-format on

#undef utl_log_define_style_merging

// Prohibit applying alignment after the color (which is necessary to make log color escaping work),
// doing it at the operator level produces the best error messages and tends to work well with LSPs
#define utl_log_prohibit_style_merging(wrapper_, style_)                                                               \
    template <class T>                                                                                                 \
    constexpr void operator|(wrapper_, style_) noexcept {                                                              \
        static_assert(always_false_v<T>, "Color modifiers should be applied after alignment.");                        \
    }                                                                                                                  \
    static_assert(true)

// clang-format off
utl_log_prohibit_style_merging(const Colored<T>& , mods::AlignRight );
utl_log_prohibit_style_merging(const Colored<T>& , mods::AlignCenter);
utl_log_prohibit_style_merging(const Colored<T>& , mods::AlignLeft  );
utl_log_prohibit_style_merging(      Colored<T>&&, mods::AlignRight );
utl_log_prohibit_style_merging(      Colored<T>&&, mods::AlignCenter);
utl_log_prohibit_style_merging(      Colored<T>&&, mods::AlignLeft  );
// clang-format on

#undef utl_log_prohibit_style_merging

} // namespace mods

// --- Public API for style modifiers ---
// --------------------------------------

[[nodiscard]] constexpr auto general(std::size_t precision = 6) noexcept {
    return mods::FloatFormat{std::chars_format::general, static_cast<int>(precision)};
}
[[nodiscard]] constexpr auto fixed(std::size_t precision = 3) noexcept {
    return mods::FloatFormat{std::chars_format::fixed, static_cast<int>(precision)};
}
[[nodiscard]] constexpr auto scientific(std::size_t precision = 3) noexcept {
    return mods::FloatFormat{std::chars_format::scientific, static_cast<int>(precision)};
}
[[nodiscard]] constexpr auto hex(std::size_t precision = 3) noexcept {
    return mods::FloatFormat{std::chars_format::hex, static_cast<int>(precision)};
}
[[nodiscard]] constexpr auto base(std::size_t base) noexcept { return mods::IntegerFormat{static_cast<int>(base)}; }
[[nodiscard]] constexpr auto align_left(std::size_t size) noexcept { return mods::AlignLeft{size}; }
[[nodiscard]] constexpr auto align_center(std::size_t size) noexcept { return mods::AlignCenter{size}; }
[[nodiscard]] constexpr auto align_right(std::size_t size) noexcept { return mods::AlignRight{size}; }

// clang-format off
namespace color {
constexpr auto black               = mods::Color{ansi::black              };
constexpr auto red                 = mods::Color{ansi::red                };
constexpr auto green               = mods::Color{ansi::green              };
constexpr auto yellow              = mods::Color{ansi::yellow             };
constexpr auto blue                = mods::Color{ansi::blue               };
constexpr auto magenta             = mods::Color{ansi::magenta            };
constexpr auto cyan                = mods::Color{ansi::cyan               };
constexpr auto white               = mods::Color{ansi::white              };
constexpr auto bright_black        = mods::Color{ansi::bright_black       };
constexpr auto bright_red          = mods::Color{ansi::bright_red         };
constexpr auto bright_green        = mods::Color{ansi::bright_green       };
constexpr auto bright_yellow       = mods::Color{ansi::bright_yellow      };
constexpr auto bright_blue         = mods::Color{ansi::bright_blue        };
constexpr auto bright_magenta      = mods::Color{ansi::bright_magenta     };
constexpr auto bright_cyan         = mods::Color{ansi::bright_cyan        };
constexpr auto bright_white        = mods::Color{ansi::bright_white       };
constexpr auto bold_black          = mods::Color{ansi::bold_black         };
constexpr auto bold_red            = mods::Color{ansi::bold_red           };
constexpr auto bold_green          = mods::Color{ansi::bold_green         };
constexpr auto bold_yellow         = mods::Color{ansi::bold_yellow        };
constexpr auto bold_blue           = mods::Color{ansi::bold_blue          };
constexpr auto bold_magenta        = mods::Color{ansi::bold_magenta       };
constexpr auto bold_cyan           = mods::Color{ansi::bold_cyan          };
constexpr auto bold_white          = mods::Color{ansi::bold_white         };
constexpr auto bold_bright_black   = mods::Color{ansi::bold_bright_black  };
constexpr auto bold_bright_red     = mods::Color{ansi::bold_bright_red    };
constexpr auto bold_bright_green   = mods::Color{ansi::bold_bright_green  };
constexpr auto bold_bright_yellow  = mods::Color{ansi::bold_bright_yellow };
constexpr auto bold_bright_blue    = mods::Color{ansi::bold_bright_blue   };
constexpr auto bold_bright_magenta = mods::Color{ansi::bold_bright_magenta};
constexpr auto bold_bright_cyan    = mods::Color{ansi::bold_bright_cyan   };
constexpr auto bold_bright_white   = mods::Color{ansi::bold_bright_white  };
} // namespace color
// clang-format on

// =================
// --- Formatter ---
// =================

// --- Member detection type traits ---
// ------------------------------------

#define utl_log_define_trait(trait_name_, ...)                                                                         \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

// clang-format off
utl_log_define_trait(has_string        , std::declval<T>().string()                              );
utl_log_define_trait(has_real          , std::declval<T>().real()                                );
utl_log_define_trait(has_imag          , std::declval<T>().imag()                                );
utl_log_define_trait(has_begin         , std::declval<T>().begin()                               );
utl_log_define_trait(has_end           , std::declval<T>().end()                                 );
utl_log_define_trait(has_input_it      , std::next(std::declval<T>().begin())                    );
utl_log_define_trait(has_get           , std::get<0>(std::declval<T>())                          );
utl_log_define_trait(has_tuple_size    , std::tuple_size<T>::value                               );
utl_log_define_trait(has_container_type, std::declval<typename std::decay_t<T>::container_type>());
utl_log_define_trait(has_rep           , std::declval<typename std::decay_t<T>::rep>()           );
utl_log_define_trait(has_period        , std::declval<typename std::decay_t<T>::period>()        );
utl_log_define_trait(has_ostream_insert, std::declval<std::ostream>() << std::declval<T>()       );
// clang-format on

#undef utl_log_define_trait

// --- Type trait chain ---
// ------------------------

// We want to provide default formatting behaviour based on type traits, however one type
// might satisfy multiple formatter-suitable type traits. To avoid ambiguity we need to select
// one based on a certain trait priority.
//
// A simple way to do it would be to have and 'if constexpr' chain inside the formatter,
// however that would leave user with no good customization points.
//
// We can emulate an 'if constexpr' chain on the type trait level by arranging them in a following pattern:
//    > is_TYPE_i = /* if TYPE should be included due to its properties         */;
//    > is_TYPE_e = /* if TYPE should be excluded due to already having a match */;
//    > is_TYPE_v = is_TYPE_i && !is_TYPE_e;
//
// Similar effect could be achieved in other ways, but this one proved to be so far the least impactful
// in terms of compile times, as we don't introduce and deep template nesting & minimize instantiations.

template <class Type>
struct Traits {
    using T = std::decay_t<Type>;

    // char types ('char')
    constexpr static bool is_char_i               = std::is_same_v<T, char>;
    constexpr static bool is_char_e               = false;
    constexpr static bool is_char_v               = is_char_i && !is_char_e;
    // enum types
    constexpr static bool is_enum_i               = std::is_enum_v<T>;
    constexpr static bool is_enum_e               = is_char_e || is_char_i;
    constexpr static bool is_enum_v               = is_enum_i && !is_enum_e;
    // types with '.string()' ('std::path')
    constexpr static bool is_path_i               = has_string_v<T>;
    constexpr static bool is_path_e               = is_enum_e || is_enum_i;
    constexpr static bool is_path_v               = is_path_i && !is_path_e;
    // string-like types ('std::string_view', 'std::string', 'const char*')
    constexpr static bool is_string_i             = std::is_convertible_v<T, std::string_view>;
    constexpr static bool is_string_e             = is_path_e || is_path_i;
    constexpr static bool is_string_v             = is_string_i && !is_string_e;
    // string-convertible types (custom classes)
    constexpr static bool is_string_convertible_i = std::is_convertible_v<T, std::string>;
    constexpr static bool is_string_convertible_e = is_string_e || is_string_i;
    constexpr static bool is_string_convertible_v = is_string_convertible_i && !is_string_convertible_e;
    // boolean types ('bool')
    constexpr static bool is_bool_i               = std::is_same_v<T, bool>;
    constexpr static bool is_bool_e               = is_string_convertible_e || is_string_convertible_i;
    constexpr static bool is_bool_v               = is_bool_i && !is_bool_e;
    // integer types ('int', 'std::uint64_t', etc.)
    constexpr static bool is_integer_i            = std::is_integral_v<T>;
    constexpr static bool is_integer_e            = is_bool_e || is_bool_i;
    constexpr static bool is_integer_v            = is_integer_i && !is_integer_e;
    // floating-point types
    constexpr static bool is_float_i              = std::is_floating_point_v<T>;
    constexpr static bool is_float_e              = is_integer_e || is_integer_i;
    constexpr static bool is_float_v              = is_float_i && !is_float_e;
    // 'std::complex'-like types
    constexpr static bool is_complex_i            = has_imag_v<T>;
    constexpr static bool is_complex_e            = is_float_e || is_float_i;
    constexpr static bool is_complex_v            = is_complex_i && !is_complex_e;
    // array-like types
    constexpr static bool is_array_i              = has_begin_v<T> && has_end_v<T> && has_input_it_v<T>;
    constexpr static bool is_array_e              = is_complex_e || is_complex_i;
    constexpr static bool is_array_v              = is_array_i && !is_array_e;
    // tuple-like types
    constexpr static bool is_tuple_i              = has_get_v<T> && has_tuple_size_v<T>;
    constexpr static bool is_tuple_e              = is_array_e || is_array_i;
    constexpr static bool is_tuple_v              = is_tuple_i && !is_tuple_e;
    // container adaptor types
    constexpr static bool is_adaptor_i            = has_container_type_v<T>;
    constexpr static bool is_adaptor_e            = is_tuple_e || is_tuple_i;
    constexpr static bool is_adaptor_v            = is_adaptor_i && !is_adaptor_e;
    // <chrono> types
    constexpr static bool is_duration_i           = has_rep_v<T> && has_period_v<T>;
    constexpr static bool is_duration_e           = is_adaptor_e || is_adaptor_i;
    constexpr static bool is_duration_v           = is_duration_i && !is_duration_e;
    // printable types
    constexpr static bool is_printable_i          = has_ostream_insert_v<T>;
    constexpr static bool is_printable_e          = is_duration_e || is_duration_i;
    constexpr static bool is_printable_v          = is_printable_i && !is_printable_e;
};

// --- String buffer ---
// ---------------------

// Simplest case of a 'buffer' concept, wraps 'std::string' into a buffer-like API for appending
// so we can use it in formatters that require an intermediate string for formatting

class StringBuffer {
    std::string& str;

public:
    StringBuffer(std::string& str) noexcept : str(str) {}

    void push_string(std::string_view sv) { this->str += sv; }

    void push_chars(std::size_t count, char ch) { this->str.append(count, ch); }
};

// --- Partial specializations ---
// -------------------------------

// Base template
template <class T, class = void>
struct Formatter {
    static_assert(always_false_v<T>, "No formatter could be deduced for the type.");
};

// char types ('char')
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_char_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_chars(1, arg);
    }
};

// enum types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_enum_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        Formatter<std::underlying_type_t<std::decay_t<T>>>{}(buffer, to_underlying(arg));
    }
};

// types with '.string()' ('std::path')
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_path_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_string(arg.string());
    }
};

// string-like types ('std::string_view', 'std::string', 'const char*')
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_string_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_string(std::string_view{arg});
    }
};

// string-convertible types (custom classes)
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_string_convertible_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_string(std::string{arg});
    }
};

// boolean types ('bool')
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_bool_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_string(arg ? "true" : "false");
    }
};

// integral types ('int', 'std::uint64_t', etc.)
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_integer_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        std::array<char, max_chars_int> res;

        const std::size_t serialized = std::to_chars(res.data(), res.data() + res.size(), arg).ptr - res.data();

        buffer.push_string(std::string_view{res.data(), serialized});
    }
};

// floating-point types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_float_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        std::array<char, max_chars_int> res;

        const std::size_t serialized = std::to_chars(res.data(), res.data() + res.size(), arg).ptr - res.data();

        buffer.push_string(std::string_view{res.data(), serialized});
    }
};

// 'std::complex'-like types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_complex_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        const auto string_formatter = Formatter<std::string_view>{};
        const auto value_formatter  = Formatter<decltype(arg.real())>{};

        value_formatter(buffer, arg.real());
        if (arg.imag() >= 0) {
            string_formatter(buffer, " + ");
            value_formatter(buffer, arg.imag());
        } else {
            string_formatter(buffer, " - ");
            value_formatter(buffer, -arg.imag());
        }
        string_formatter(buffer, "i");
    }
};

// array-like types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_array_v>> {
    static constexpr std::string_view prefix    = "[ ";
    static constexpr std::string_view suffix    = " ]";
    static constexpr std::string_view delimiter = ", ";

    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        const auto string_formatter = Formatter<std::string_view>{};
        const auto value_formatter  = Formatter<typename std::decay_t<T>::value_type>{};

        string_formatter(buffer, prefix);
        if (arg.begin() != arg.end()) {
            for (auto it = arg.begin();;) {
                value_formatter(buffer, *it);
                if (++it == arg.end()) break; // prevents trailing comma
                string_formatter(buffer, delimiter);
            }
        }
        string_formatter(buffer, suffix);
    }
};

// tuple-like types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_tuple_v>> {
    static constexpr std::string_view prefix    = "< ";
    static constexpr std::string_view suffix    = " >";
    static constexpr std::string_view delimiter = ", ";

    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        const auto string_formatter = Formatter<std::string_view>{};

        string_formatter(buffer, prefix);

        for_sequence(std::make_index_sequence<std::tuple_size_v<T>>{}, [&](auto index) {
            const auto& element = std::get<index>(arg); // 'index' is an 'std::integral_constant<std::size_t, i>'

            if constexpr (index != 0) string_formatter(buffer, delimiter);

            Formatter<std::tuple_element_t<index, T>>{}(buffer, element);
        });

        string_formatter(buffer, suffix);
    }
};

// container adaptor types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_adaptor_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        const auto& ref = underlying_container_cref(arg);

        Formatter<std::decay_t<decltype(ref)>>{}(buffer, ref);
    }
};

// <chrono> types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_duration_v>> {
    constexpr static std::size_t relevant_units = 3;

    constexpr static std::array<std::string_view, SplitDuration::size> names = {"hours", "min", "sec",
                                                                                "ms",    "us",  "ns"};

    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {

        auto string_formatter  = Formatter<std::string_view>{};
        auto integer_formatter = Formatter<SplitDuration::common_rep>{};

        // Takes 'unit_count' of the highest relevant units and converts them to string,
        // for example with 'unit_count' equal to '3', we will have:
        //
        // timescale <= hours   =>   show { hours, min, sec }   =>   string "___ hours ___ min ___ sec"
        // timescale <= min     =>   show {   min, sec,  ms }   =>   string "___ min ___ sec ___ ms"
        // timescale <= sec     =>   show {   sec,  ms,  us }   =>   string "___ sec ___ ms ___ us"
        // timescale <= ms      =>   show {    ms,  us,  ns }   =>   string "___ ms ___ us ___ ns"
        // timescale <= us      =>   show {    us,  ns      }   =>   string "___ us ___ ns"
        // timescale <= ns      =>   show {    ns           }   =>   string "___ ns"

        const std::array<SplitDuration::common_rep, SplitDuration::size> counts = unit_split(arg).count();

        for (std::size_t unit = 0; unit < counts.size(); ++unit) {
            if (counts[unit]) {
                const std::size_t last =
                    (unit + relevant_units < counts.size()) ? (unit + relevant_units) : counts.size();
                // don't want to include the whole <algorithm> just for 'std::max()'

                for (std::size_t k = unit; k < last; ++k) {
                    integer_formatter(buffer, counts[k]);
                    string_formatter(buffer, " ");
                    string_formatter(buffer, names[k]);
                    if (k + 1 != last) string_formatter(buffer, " "); // prevents trailing space at the end
                }
                return;
            }
        }

        string_formatter(buffer, "0 ns"); // fallback, unlikely to ever be triggered
    }
};

// printable types
template <class T>
struct Formatter<T, std::enable_if_t<Traits<T>::is_printable_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) const {
        buffer.push_string((std::ostringstream{} << arg).str());
        // creating a string stream every time is slow, but there is no way around it,
        // this is simply a flaw of streams as a design
    }
};

// 'FormattedFloat<T>' types
template <class T>
struct Formatter<FormattedFloat<T>, std::enable_if_t<Traits<T>::is_float_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const FormattedFloat<T>& arg) const {
        std::array<char, max_chars_int> res;

        const std::size_t serialized =
            std::to_chars(res.data(), res.data() + res.size(), arg.value, arg.mod.format, arg.mod.precision).ptr -
            res.data();

        buffer.push_string(std::string_view{res.data(), serialized});
    }
};

// 'FormattedInteger<T>' types
template <class T>
struct Formatter<FormattedInteger<T>, std::enable_if_t<Traits<T>::is_integer_v>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const FormattedInteger<T>& arg) const {
        std::array<char, max_chars_int> res;

        const std::size_t serialized =
            std::to_chars(res.data(), res.data() + res.size(), arg.value, arg.mod.base).ptr - res.data();

        buffer.push_string(std::string_view{res.data(), serialized});
    }
};

// 'AlignedLeft<T>' types
template <class T>
struct Formatter<AlignedLeft<T>, void> {
    template <class Buffer>
    void operator()(Buffer& buffer, const AlignedLeft<T>& arg) const {
        std::string& temp = thread_local_temporary_string();
        StringBuffer temp_buffer(temp);

        Formatter<T>{}(temp_buffer, arg.value);

        const std::size_t size_no_pad   = temp.size();
        const std::size_t size_with_pad = max(arg.mod.size, size_no_pad);
        const std::size_t right_pad     = size_with_pad - size_no_pad;

        buffer.push_string(temp);
        buffer.push_chars(right_pad, ' ');
    }
};

// 'AlignedCenter<T>' types
template <class T>
struct Formatter<AlignedCenter<T>, void> {
    template <class Buffer>
    void operator()(Buffer& buffer, const AlignedCenter<T>& arg) const {
        std::string& temp = thread_local_temporary_string();
        StringBuffer temp_buffer(temp);

        Formatter<T>{}(temp_buffer, arg.value);

        const std::size_t size_no_pad   = temp.size();
        const std::size_t size_with_pad = max(arg.mod.size, size_no_pad);
        const std::size_t left_pad      = (size_with_pad - size_no_pad) / 2;
        const std::size_t right_pad     = size_with_pad - size_no_pad - left_pad;

        buffer.push_chars(left_pad, ' ');
        buffer.push_string(temp);
        buffer.push_chars(right_pad, ' ');
    }
};

// 'AlignedRight<T>' types
template <class T>
struct Formatter<AlignedRight<T>, void> {
    template <class Buffer>
    void operator()(Buffer& buffer, const AlignedRight<T>& arg) const {
        std::string& temp = thread_local_temporary_string();
        StringBuffer temp_buffer(temp);

        Formatter<T>{}(temp_buffer, arg.value);

        const std::size_t size_no_pad   = temp.size();
        const std::size_t size_with_pad = max(arg.mod.size, size_no_pad);
        const std::size_t left_pad      = size_with_pad - size_no_pad;

        buffer.push_chars(left_pad, ' ');
        buffer.push_string(temp);
    }
};

// 'Colored<T>' types
template <class T>
struct Formatter<Colored<T>, void> {
    template <class Buffer>
    void operator()(Buffer& buffer, const Colored<T>& arg) const {
        buffer.push_string(arg.mod.code);
        Formatter<T>{}(buffer, arg.value);
        buffer.push_string(ansi::reset);
    }
};

// ==============
// --- Logger ---
// ==============

// Class layout:
//
//    Logger               | Input: Message    | Output: Message    | - Captures meta, forwards message to all sinks
//    -> tuple<Sink, ...>
//
//    Sink                 | Input: Message    | Output: Message    | - Wraps underlying API & provides defaults
//    -> Protector         | Input: Message    | Output: Message    | - Handles safe/unsafe threading
//       -> Writer         | Input: Message    | Output: Log string | - Handles message formatting
//         -> Buffer       | Input: Log string | Output: Log string | - Handles instant/fixed/timed buffering
//            -> Flusher   | Input: Log string | Output: Log string | - Handles sync/async flushing
//               -> Output | Input: Log string | Output: Log string | - Wraps underlying IO output
//
// Specific instances of all listed components depend on specializations selected though policies.

// --- Message metadata ---
// ------------------------

using Clock = std::chrono::steady_clock;

// Metadata associated with a logging record, generated once by the 'Logger' and distributed to all sinks
struct Record {
    Clock::duration  elapsed;
    std::string_view file;
    std::size_t      line;
};

// --- Policies ---
// ----------------

namespace policy {

enum class Type { FILE, STREAM };

enum class Level { ERR = 0, WARN = 1, NOTE = 2, INFO = 3, DEBUG = 4, TRACE = 5 };

enum class Color { NONE, ANSI };

enum class Format {
    DATE     = 1 << 0,
    TITLE    = 1 << 1,
    THREAD   = 1 << 2,
    UPTIME   = 1 << 3,
    CALLSITE = 1 << 4,
    LEVEL    = 1 << 5,
    NONE     = 0,
    FULL     = DATE | TITLE | THREAD | UPTIME | CALLSITE | LEVEL
}; // bitmask enum

[[nodiscard]] constexpr Format operator|(Format a, Format b) noexcept {
    return static_cast<Format>(to_underlying(a) | to_underlying(b));
}

[[nodiscard]] constexpr Format operator&(Format a, Format b) noexcept {
    return static_cast<Format>(to_underlying(a) & to_underlying(b));
}

enum class Buffering { NONE, FIXED, TIMED };

enum class Flushing { SYNC, ASYNC };

enum class Threading { UNSAFE, SAFE };

} // namespace policy

// =========================
// --- Component: Output ---
// =========================

template <policy::Type type>
class Output;

// --- File output ---
// -------------------

template <>
class Output<policy::Type::FILE> {
    std::ofstream file;

public:
    Output(const std::string& name) : file(name) {}
    Output(std::ofstream&& file) : file(std::move(file)) {}

    void flush_string(std::string_view sv) {
        this->file.write(sv.data(), sv.size());
        this->file.flush();
    }

    void flush_chars(std::size_t count, char ch) {
        std::ostreambuf_iterator<char> first(this->file);
        for (std::size_t i = 0; i < count; ++i) first = ch;
        // fastest way of writing N chars to a stream, 'std::ostreambuf_iterator' writes character to a stream
        // when assigned, dereference/increment is no-op making it rather strange for an "iterator"
    }
};

// --- Stream output ---
// ---------------------

template <>
class Output<policy::Type::STREAM> {
    std::ostream& os;

public:
    Output(std::ostream& os) noexcept : os(os) {}

    void flush_string(std::string_view sv) {
        this->os.write(sv.data(), sv.size());
        this->os.flush();
    }

    void flush_chars(std::size_t count, char ch) {
        std::ostreambuf_iterator<char> first(this->os);
        for (std::size_t i = 0; i < count; ++i) first = ch;
        // fastest way of writing N chars to a stream, 'std::ostreambuf_iterator' writes character to a stream
        // when assigned, dereference/increment is no-op making it a rather strange case of an "iterator"
    }
};

// ==========================
// --- Component: Flusher ---
// ==========================

template <class OutputType, policy::Flushing flushing>
class Flusher;

// --- Synchonous flushing ---
// ---------------------------

template <class OutputType>
class Flusher<OutputType, policy::Flushing::SYNC> {
    OutputType output;

public:
    Flusher(OutputType&& output) : output(std::move(output)) {}

    void flush_string(std::string_view sv) { this->output.flush_string(sv); }

    void flush_chars(std::size_t count, char ch) { this->output.flush_chars(count, ch); }
};

// --- Async flushing ---
// ----------------------

template <class OutputType>
class Flusher<OutputType, policy::Flushing::ASYNC> {
    OutputType output; // destruction order matters here, 'output' should be available until 'worker' thread joins

    std::unique_ptr<WorkerThread> worker = std::make_unique<WorkerThread>();

public:
    Flusher(OutputType&& output) : output(std::move(output)) {}

    Flusher(Flusher&& other) : output(std::move(other.output)), worker(std::move(other.worker)) {}

    void flush_string(std::string_view sv) {
        this->worker->detached_task([&out = output, str = std::string(sv)]() { out.flush_string(str); });

        // Note 1: The buffer might be mutated while the other thread flushes it, we have to pass a copy
        // Note 2: 'std::bind' doesn't bind reference arguments, we have to add a wrapper with 'std::ref'
    }

    void flush_chars(std::size_t count, char ch) {
        this->worker->detached_task([&out = output, count, ch] { out.flush_chars(count, ch); });
    }
};

// =========================
// --- Component: Buffer ---
// =========================

template <class FlusherType, policy::Buffering buffering>
class Buffer;

// --- Instant buffering ---
// -------------------------

template <class FlusherType>
class Buffer<FlusherType, policy::Buffering::NONE> {
    FlusherType flusher;

public:
    Buffer(FlusherType&& flusher) : flusher(std::move(flusher)) {}

    void push_record(const Record&) const noexcept {} // only matters for timed buffer

    void push_string(std::string_view sv) { this->flusher.flush_string(sv); }

    void push_chars(std::size_t count, char ch) { this->flusher.flush_chars(count, ch); }
};

// --- Fixed buffering ---
// -----------------------

template <class FlusherType>
class Buffer<FlusherType, policy::Buffering::FIXED> {
    constexpr static std::size_t size = buffering_size;

    FlusherType            flusher;
    std::array<char, size> buffer{};
    std::size_t            cursor{};

public:
    Buffer(FlusherType&& output) : flusher(std::move(output)) {}

    // Buffered flusher need non-trivial destructor and move semantics to ensure correct flushing of
    // the remaining buffer upon destruction. Moved-from buffer should not flush upon destruction.
    Buffer(Buffer&& other) : flusher(std::move(other.flusher)), buffer(other.buffer), cursor(other.cursor) {
        other.cursor = size;
    }

    ~Buffer() {
        if (this->cursor == size) return;

        this->flusher.flush_string(std::string_view{this->buffer.data(), this->cursor});
    }

    void push_record(const Record&) const noexcept {} // only matters for timed buffer

    void push_string(std::string_view sv) {
        while (true) {
            const std::size_t space_needed    = sv.size();
            const std::size_t space_remaining = size - this->cursor;

            // Fast path: Message fits into the buffer
            if (space_needed <= space_remaining) {
                for (std::size_t i = 0; i < space_needed; ++i) this->buffer[this->cursor + i] = sv[i];
                this->cursor += space_needed;

                return;
            }
            // Slow path: Message doesn't fit, need to flush
            else {
                for (std::size_t i = 0; i < space_remaining; ++i) this->buffer[this->cursor + i] = sv[i];
                this->cursor = 0;
                this->flusher.flush_string(std::string_view{this->buffer.data(), this->buffer.size()});

                sv.remove_prefix(space_remaining);
            }
        }
    }

    void push_chars(std::size_t count, char ch) {
        while (true) {
            const std::size_t space_needed    = count;
            const std::size_t space_remaining = size - this->cursor;

            // Fast path: Message fits into the buffer
            if (space_needed <= space_remaining) {
                for (std::size_t i = 0; i < space_needed; ++i) this->buffer[this->cursor + i] = ch;
                this->cursor += space_needed;

                return;
            }
            // Slow path: Message doesn't fit, need to flush
            else {
                for (std::size_t i = 0; i < space_remaining; ++i) this->buffer[this->cursor + i] = ch;
                this->cursor = 0;
                this->flusher.flush_string(std::string_view{this->buffer.data(), this->buffer.size()});

                count -= space_remaining; // guaranteed to not underflow
            }
        }
    }
};

// --- Timed buffering ---
// -----------------------

template <class FlusherType>
class Buffer<FlusherType, policy::Buffering::TIMED> {
    FlusherType     flusher;
    std::string     buffer;
    Clock::duration last_flush_uptime{};

    void flush() {
        this->flusher.flush_string(this->buffer);

        this->buffer.clear();
        // standard doesn't guarantee that this doesn't reallocate (unlike 'std::vector<>'), however all
        // existing implementations do the reasonable thing and don't reallocate so we can mostly rely on it
    }

public:
    Buffer(FlusherType&& flusher) : flusher(std::move(flusher)) {}

    Buffer(Buffer&& other)
        : flusher(std::move(other).flusher), buffer(std::move(other).buffer),
          last_flush_uptime(other.last_flush_uptime) {
        other.buffer.clear(); // ensures moved-from buffer will not flush in destructor
    }

    ~Buffer() {
        if (!this->buffer.empty()) this->flush();
    }

    void push_record(const Record& record) noexcept {
        // retrieving timestamps is expensive, we can reuse the one already produced by the logger

        if (record.elapsed - this->last_flush_uptime > buffering_time) {
            this->flush();
            this->last_flush_uptime = record.elapsed;
        }
    }

    void push_string(std::string_view sv) { this->buffer += sv; }

    void push_chars(std::size_t count, char ch) { this->buffer.append(count, ch); }
};

// =========================
// --- Component: Writer ---
// =========================

// --- Style configuration ---
// ---------------------------

namespace config {

constexpr std::size_t width_thread        = 6;
constexpr std::size_t width_uptime        = 8;
constexpr std::size_t width_callsite_name = 24; // split used for file:line alignment
constexpr std::size_t width_callsite_line = 4;
constexpr std::size_t width_callsite      = width_callsite_name + 1 + width_callsite_line;
constexpr std::size_t width_level         = 5;
constexpr std::size_t width_message       = 30; // doesn't limit actual message size, used for separator width

constexpr std::string_view delimiter_front = "| ";
constexpr std::string_view delimiter_mid   = " | ";

constexpr std::string_view title_thread   = "thread";
constexpr std::string_view title_uptime   = "  uptime";
constexpr std::string_view title_callsite = "                     callsite";
constexpr std::string_view title_level    = "level";
constexpr std::string_view title_message  = "message";

constexpr std::string_view date_prefix = "date -> ";

constexpr char hline_fill = '-';

constexpr std::string_view line_break = "\n";

constexpr std::string_view name_err   = "  ERR";
constexpr std::string_view name_warn  = " WARN";
constexpr std::string_view name_note  = " NOTE";
constexpr std::string_view name_info  = " INFO";
constexpr std::string_view name_debug = "DEBUG";
constexpr std::string_view name_trace = "TRACE";

constexpr std::string_view color_header = ansi::bold_cyan;

constexpr std::string_view color_err   = ansi::bold_red;
constexpr std::string_view color_warn  = ansi::yellow;
constexpr std::string_view color_note  = ansi::magenta;
constexpr std::string_view color_info  = ansi::white;
constexpr std::string_view color_debug = ansi::green;
constexpr std::string_view color_trace = ansi::bright_black;

static_assert(width_thread == title_thread.size());
static_assert(width_uptime == title_uptime.size());
static_assert(width_callsite == title_callsite.size());
static_assert(width_level == title_level.size());
static_assert(width_message > title_message.size());

static_assert(width_level == name_trace.size());
static_assert(width_level == name_debug.size());
static_assert(width_level == name_info.size());
static_assert(width_level == name_note.size());
static_assert(width_level == name_warn.size());
static_assert(width_level == name_err.size());

} // namespace config

// --- Component ---
// -----------------

// Component that wraps 'Formatter' with sink-specific formatting

template <class BufferType, policy::Level level, policy::Color color, policy::Format format>
struct Writer {
private:
    BufferType buffer;

    constexpr static bool has_color = color == policy::Color::ANSI;

    constexpr static bool format_date  = to_bool(format & policy::Format::DATE);
    constexpr static bool format_title = to_bool(format & policy::Format::TITLE);

    constexpr static auto delimiter_date = config::delimiter_front;

    constexpr static bool format_thread   = to_bool(format & policy::Format::THREAD);
    constexpr static bool format_uptime   = to_bool(format & policy::Format::UPTIME);
    constexpr static bool format_callsite = to_bool(format & policy::Format::CALLSITE);
    constexpr static bool format_level    = to_bool(format & policy::Format::LEVEL);
    constexpr static bool format_message  = true;

    constexpr static bool front_is_thread   = format_thread;
    constexpr static bool front_is_uptime   = format_uptime && !front_is_thread;
    constexpr static bool front_is_callsite = format_callsite && !front_is_thread && !front_is_uptime;
    constexpr static bool front_is_level = format_level && !front_is_thread && !front_is_uptime && !front_is_callsite;
    constexpr static bool front_is_message =
        format_message && !front_is_thread && !front_is_uptime && !front_is_callsite && !front_is_level;

    constexpr static auto delimiter_thread   = front_is_thread ? config::delimiter_front : config::delimiter_mid;
    constexpr static auto delimiter_uptime   = front_is_uptime ? config::delimiter_front : config::delimiter_mid;
    constexpr static auto delimiter_callsite = front_is_callsite ? config::delimiter_front : config::delimiter_mid;
    constexpr static auto delimiter_level    = front_is_level ? config::delimiter_front : config::delimiter_mid;
    constexpr static auto delimiter_message  = front_is_message ? config::delimiter_front : config::delimiter_mid;

    void write_thread() {
        using styled_type     = AlignedLeft<int>;
        const styled_type arg = this_thread_linear_id() | align_left(config::width_thread);

        Formatter<styled_type>{}(this->buffer, arg);
    }

    void write_uptime(const Record& record) {
        using styled_type     = AlignedRight<FormattedFloat<double>>;
        const styled_type arg = Sec(record.elapsed).count() | fixed(2) | align_right(config::width_uptime);

        Formatter<styled_type>{}(this->buffer, arg);
    }

    void write_callsite([[maybe_unused]] const Record& record) {
        using styled_file      = AlignedRight<const std::string_view&>;
        const styled_file file = record.file | align_right(config::width_callsite_name);

        using styled_line      = AlignedLeft<const std::size_t&>;
        const styled_line line = record.line | align_left(config::width_callsite_line);

        Formatter<styled_file>{}(this->buffer, file);
        Formatter<char>{}(this->buffer, ':');
        Formatter<styled_line>{}(this->buffer, line);
    }

    template <policy::Level message_level>
    void write_level() {
        // clang-format off
        if constexpr (message_level == policy::Level::ERR  ) this->buffer.push_string(config::name_err  );
        if constexpr (message_level == policy::Level::WARN ) this->buffer.push_string(config::name_warn );
        if constexpr (message_level == policy::Level::NOTE ) this->buffer.push_string(config::name_note );
        if constexpr (message_level == policy::Level::INFO ) this->buffer.push_string(config::name_info );
        if constexpr (message_level == policy::Level::DEBUG) this->buffer.push_string(config::name_debug);
        if constexpr (message_level == policy::Level::TRACE) this->buffer.push_string(config::name_trace);
        // clang-format on
    }

    template <policy::Level message_level>
    void write_color_message() {
        // clang-format off
        if constexpr (message_level == policy::Level::ERR  ) this->buffer.push_string(config::color_err  );
        if constexpr (message_level == policy::Level::WARN ) this->buffer.push_string(config::color_warn );
        if constexpr (message_level == policy::Level::NOTE ) this->buffer.push_string(config::color_note );
        if constexpr (message_level == policy::Level::INFO ) this->buffer.push_string(config::color_info );
        if constexpr (message_level == policy::Level::DEBUG) this->buffer.push_string(config::color_debug);
        if constexpr (message_level == policy::Level::TRACE) this->buffer.push_string(config::color_trace);
        // clang-format on
    }

    void write_color_header() { this->buffer.push_string(config::color_header); }

    void write_color_reset() { this->buffer.push_string(ansi::reset); }

    template <policy::Level message_level, class T>
    void write_arg(const T& arg) {
        Formatter<T>{}(this->buffer, arg);
    }

    // Color modifier requires special handling at the logger level since we need to properly escape & restore
    // current logging level color. This wouldn't be required if ANSI codes could be nested.
    template <policy::Level message_level, class T>
    void write_arg(const Colored<T>& arg) {
        // Switch to message color
        if constexpr (has_color) this->write_color_reset();
        if constexpr (has_color) this->buffer.push_string(arg.mod.code);

        this->write_arg<message_level>(arg.value);

        // Restore logger color
        if constexpr (has_color) this->buffer.push_string(ansi::reset);
        if constexpr (has_color) this->write_color_message<message_level>();
    }

    void write_header_datetime() {
        this->buffer.push_string(config::delimiter_front);
        this->buffer.push_string(config::date_prefix);
        this->buffer.push_string(datetime_string());
        this->buffer.push_string(config::line_break);
    }

    void write_header_separator() {
        if constexpr (format_thread) this->buffer.push_string(delimiter_thread);
        if constexpr (format_thread) this->buffer.push_chars(config::width_thread, config::hline_fill);
        if constexpr (format_uptime) this->buffer.push_string(delimiter_uptime);
        if constexpr (format_uptime) this->buffer.push_chars(config::width_uptime, config::hline_fill);
        if constexpr (format_callsite) this->buffer.push_string(delimiter_callsite);
        if constexpr (format_callsite) this->buffer.push_chars(config::width_callsite, config::hline_fill);
        if constexpr (format_level) this->buffer.push_string(delimiter_level);
        if constexpr (format_level) this->buffer.push_chars(config::width_level, config::hline_fill);
        this->buffer.push_string(delimiter_message);
        this->buffer.push_chars(config::width_message, config::hline_fill);
        this->buffer.push_string(config::line_break);
    }

    void write_header_hline() {
        constexpr std::size_t w_thread   = format_thread ? (config::width_thread + delimiter_thread.size()) : 0;
        constexpr std::size_t w_uptime   = format_uptime ? (config::width_uptime + delimiter_uptime.size()) : 0;
        constexpr std::size_t w_callsite = format_callsite ? (config::width_callsite + delimiter_callsite.size()) : 0;
        constexpr std::size_t w_level    = format_level ? (config::width_level + delimiter_level.size()) : 0;
        constexpr std::size_t w_message  = config::width_message + delimiter_message.size();

        constexpr std::size_t hline_total =
            w_thread + w_uptime + w_callsite + w_level + w_message - config::delimiter_front.size();

        this->buffer.push_string(config::delimiter_front);
        this->buffer.push_chars(hline_total, config::hline_fill);
        this->buffer.push_string(config::line_break);
    }

    void write_header_title() {
        if constexpr (format_thread) this->buffer.push_string(delimiter_thread);
        if constexpr (format_thread) this->buffer.push_string(config::title_thread);
        if constexpr (format_uptime) this->buffer.push_string(delimiter_uptime);
        if constexpr (format_uptime) this->buffer.push_string(config::title_uptime);
        if constexpr (format_callsite) this->buffer.push_string(delimiter_callsite);
        if constexpr (format_callsite) this->buffer.push_string(config::title_callsite);
        if constexpr (format_level) this->buffer.push_string(delimiter_level);
        if constexpr (format_level) this->buffer.push_string(config::title_level);
        this->buffer.push_string(delimiter_message);
        this->buffer.push_string(config::title_message);
        this->buffer.push_string(config::line_break);
    }

    void write_header() {
        // Start color
        if constexpr (has_color) this->write_color_header();

        if constexpr (format_date) {
            this->write_header_hline();
            this->write_header_datetime();
            this->write_header_separator();
        }

        if constexpr (format_title) {
            this->write_header_title();
            this->write_header_separator();
        }

        // End color
        if constexpr (has_color) this->write_color_reset();
    }

    template <policy::Level message_level, class... Args>
    void write_message(const Record& record, const Args&... args) {
        // Start color
        if constexpr (has_color) this->write_color_message<message_level>();

        // Format info & message
        if constexpr (format_thread) this->buffer.push_string(delimiter_thread);
        if constexpr (format_thread) this->write_thread();
        if constexpr (format_uptime) this->buffer.push_string(delimiter_uptime);
        if constexpr (format_uptime) this->write_uptime(record);
        if constexpr (format_callsite) this->buffer.push_string(delimiter_callsite);
        if constexpr (format_callsite) this->write_callsite(record);
        if constexpr (format_level) this->buffer.push_string(delimiter_level);
        if constexpr (format_level) this->write_level<message_level>();

        this->buffer.push_string(delimiter_message);
        (this->write_arg<message_level>(args), ...);
        this->buffer.push_string(config::line_break);

        // Notify buffer of the record metadata it can potentially use
        this->buffer.push_record(record);

        // End color
        if constexpr (has_color) this->write_color_reset();
    }

public:
    Writer(BufferType&& buffer) : buffer(std::move(buffer)) {}

    void header() {
        if constexpr (format_date || format_title) this->write_header();
    }

    template <policy::Level message_level, class... Args>
    void message([[maybe_unused]] const Record& record, [[maybe_unused]] const Args&... args) {
        if constexpr (message_level <= level) this->write_message<message_level>(record, args...);
        // Note: Both '[[maybe_unused]]' and splitting 'write_message()' into a separate method are
        //       necessary to prevent MSVC from complaining about unused code at W4 warning level
        //       in cases where this methods should intentionally compile to nothing.
    }
};

// ============================
// --- Component: Protector ---
// ============================

template <class WriterType, policy::Threading>
class Protector;

// --- Thread-unsafe writing ---
// -----------------------------

template <class WriterType>
class Protector<WriterType, policy::Threading::UNSAFE> {
    WriterType writer;

public:
    Protector(WriterType&& writer) : writer(std::move(writer)) {}

    void header() { this->writer.header(); }

    template <policy::Level message_level, class... Args>
    void message(const Record& record, const Args&... args) {
        this->writer.template message<message_level>(record, args...);
    }
};

// --- Thread-safe writing ---
// ---------------------------

template <class WriterType>
class Protector<WriterType, policy::Threading::SAFE> {
    WriterType writer;
    std::mutex mutex;

public:
    Protector(WriterType&& writer) : writer(std::move(writer)) {}

    Protector(Protector&& other) : writer(std::move(other.writer)) {}
    // we assume move to be thread-safe since it should only be done in logger constructor which is thread-safe
    // by itself due being either 'static' or function-local, otherwise we'd need to lock 'other.mutex'

    void header() { this->writer.header(); }

    template <policy::Level message_level, class... Args>
    void message(const Record& record, const Args&... args) {
        const std::lock_guard lock(this->mutex);
        this->writer.template message<message_level>(record, args...);
    }
};

// =======================
// --- Component: Sink ---
// =======================

// Component that wraps all the previous components into a public API with defaults & CTAD

// clang-format off
template <
    policy::Type      type,
    policy::Level     level     = (type == policy::Type::STREAM) ? policy::Level::INFO : policy::Level::TRACE,
    policy::Color     color     = (type == policy::Type::STREAM) ? policy::Color::ANSI : policy::Color::NONE,
    policy::Format    format    = policy::Format::FULL,
    policy::Buffering buffering = (type == policy::Type::STREAM) ? policy::Buffering::NONE : policy::Buffering::FIXED,
    policy::Flushing  flushing  = policy::Flushing::SYNC,
    policy::Threading threading = policy::Threading::SAFE
>
// clang-format on
class Sink {
    using output_type    = Output<type>;
    using flusher_type   = Flusher<output_type, flushing>;
    using buffer_type    = Buffer<flusher_type, buffering>;
    using writer_type    = Writer<buffer_type, level, color, format>;
    using protector_type = Protector<writer_type, threading>;

    protector_type protector;

public:
    Sink(protector_type&& protector) : protector(std::move(protector)) {}

    void header() { this->protector.header(); }

    template <policy::Level message_level, class... Args>
    void message(const Record& record, const Args&... args) {
        this->protector.template message<message_level>(record, args...);
    }

    // Stream sink preset
    Sink(std::ostream& os) : Sink(protector_type(writer_type(buffer_type(flusher_type(output_type(os)))))) {}
    // File sink preset
    Sink(std::ofstream&& file)
        : Sink(protector_type(writer_type(buffer_type(flusher_type(output_type(std::move(file))))))) {}
    Sink(std::string_view name)
        : Sink(protector_type(writer_type(buffer_type(flusher_type(output_type(std::string(name))))))) {}
};

// CTAD for presets
Sink(std::ostream&) -> Sink<policy::Type::STREAM>;
Sink(std::ofstream&&) -> Sink<policy::Type::FILE>;
Sink(std::string_view) -> Sink<policy::Type::FILE>;

// =========================
// --- Component: Logger ---
// =========================

// --- Codegen macros ---
// ----------------------

// Since we want to have macro-free API on the user side, the only way of capturing callsite is
// 'std::source_location' or its re-implementation. The only way to capture callsite with such class
// in C++17 is by using it as a defaulted function parameter, however we cannot have defaulted parameters
// after a variadic pack (which is what logging functions accept).
//
// Some loggers that use global logging functions work around this by replacing such functions with class constructor,
// that use special CTAD to omit the need to pass source location. However, we cannot use this with a local logger API
// without making it weird for the user. The only way to achieve the desired regular syntax is to manually provide
// overloads for every number of arguments up to a certain large N. Since doing that truly manually would require
// an unreasonable amount of code repetition, we use macros to generate those function with preprocessor.
//
// This is truly horrible, but sacrifices must be made if we want a nice user API.
//
// Such codegen could also be implemented in a more concise way using map-macro, however this adds a lot of
// nested preprocessing which bloats the compile time and slows down LSPs, so we use simple & shallow macros
// even through it requires more boilerplate on the use site, this results in no measurable slowdown.

#define utl_log_hold(...) __VA_ARGS__

#define utl_log_member_alias(template_params_, function_params_, args_)                                                \
    template <template_params_>                                                                                        \
    void err(function_params_, SourceLocation location = SourceLocation::current()) {                                  \
        this->message<policy::Level::ERR>(location, args_);                                                            \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void warn(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        this->message<policy::Level::WARN>(location, args_);                                                           \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void note(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        this->message<policy::Level::NOTE>(location, args_);                                                           \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void info(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        this->message<policy::Level::INFO>(location, args_);                                                           \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void debug(function_params_, SourceLocation location = SourceLocation::current()) {                                \
        this->message<policy::Level::DEBUG>(location, args_);                                                          \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void trace(function_params_, SourceLocation location = SourceLocation::current()) {                                \
        this->message<policy::Level::TRACE>(location, args_);                                                          \
    }

#define utl_log_function_alias(template_params_, function_params_, args_)                                              \
    template <template_params_>                                                                                        \
    void err(function_params_, SourceLocation location = SourceLocation::current()) {                                  \
        default_logger().err(args_, location);                                                                         \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void warn(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        default_logger().warn(args_, location);                                                                        \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void note(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        default_logger().note(args_, location);                                                                        \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void info(function_params_, SourceLocation location = SourceLocation::current()) {                                 \
        default_logger().info(args_, location);                                                                        \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void debug(function_params_, SourceLocation location = SourceLocation::current()) {                                \
        default_logger().debug(args_, location);                                                                       \
    }                                                                                                                  \
    template <template_params_>                                                                                        \
    void trace(function_params_, SourceLocation location = SourceLocation::current()) {                                \
        default_logger().trace(args_, location);                                                                       \
    }

// --- Component ---
// -----------------

// Component that wraps a number of sinks and distributes records to them

template <class... Sinks>
class Logger {
    std::tuple<Sinks...> sinks;
    Clock::time_point    creation_time_point = Clock::now();

    template <policy::Level message_level, class... Args>
    void message(SourceLocation location, const Args&... args) {
        // Get record info
        Record record;
        record.elapsed = Clock::now() - this->creation_time_point;

        std::string_view path = location.function_name();

        record.file = path.substr(path.find_last_of("\\/") + 1);
        record.line = static_cast<std::size_t>(location.line());

        // Note 1: Even if there is nothing to trim, the 'file' will be correct due to the 'npos + 1 == 0' wrap-around

        // Note 2: We have no choice, but to evaluate 'std::source_location' at runtime due to a deficiency in its
        //         design. In C++20 we do get potential constexpr source location through non-type template parameters,
        //         but even we have to implement a custom source location using compiler build-ins since standard one
        //         doesn't work as a non-type parameter.

        // Forward record & message to every sink
        tuple_for_each(this->sinks, [&](auto&& sink) { sink.template message<message_level>(record, args...); });
    }

public:
    Logger(Sinks&&... sinks) : sinks(std::move(sinks)...) {
        tuple_for_each(this->sinks, [&](auto&& sink) { sink.header(); }); // [Important!]
        // any buffer operations should happen AFTER the sink construction, since during construction
        // buffer & output pointers can change, which would break the async case (single-threaded case is fine)
    }

    // Create err() / warn() / note() / info() / debug() / trace() for up to 18 arguments
    // clang-format off
    utl_log_member_alias( // 1
        utl_log_hold(class A   ),
        utl_log_hold(const A& a),
        utl_log_hold(         a)
    )
    utl_log_member_alias( // 2
        utl_log_hold(class A   , class B   ),
        utl_log_hold(const A& a, const B& b),
        utl_log_hold(         a,          b)
    )
    utl_log_member_alias( // 3
        utl_log_hold(class A   , class B   , class C   ),
        utl_log_hold(const A& a, const B& b, const C& c),
        utl_log_hold(         a,          b,          c)
    )
    utl_log_member_alias( // 4
        utl_log_hold(class A   , class B   , class C   , class D   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d),
        utl_log_hold(         a,          b,          c,          d)
    )
    utl_log_member_alias( // 5
        utl_log_hold(class A   , class B   , class C   , class D   , class E   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e),
        utl_log_hold(         a,          b,          c,          d,          e)
    )
    utl_log_member_alias( // 6
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f),
        utl_log_hold(         a,          b,          c,          d,          e,          f)
    )
    utl_log_member_alias( // 7
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g)
    )
    utl_log_member_alias( // 8
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h)
    )
    utl_log_member_alias( // 9
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i)
    )
    utl_log_member_alias( // 10
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J                                                                                                   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j                                                                                                ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j                                                                                                )
    )
    utl_log_member_alias( // 11
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K                                                                                       ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k                                                                                    ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k                                                                                    )
    )
    utl_log_member_alias( // 12
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L                                                                           ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l                                                                        ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l                                                                        )
    )
    utl_log_member_alias( // 13
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M                                                               ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m                                                            ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m                                                            )
    )
    utl_log_member_alias( // 14
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M   , class N                                                   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m, const N& n                                                ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m,          n                                                )
    )
    utl_log_member_alias( // 15
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M   , class N   , class O                                       ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m, const N& n, const O& o                                    ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m,          n,          o                                    )
    )
    utl_log_member_alias( // 16
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M   , class N   , class O   , class P                           ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p                        ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m,          n,          o,          p                        )
    )
    utl_log_member_alias( // 17
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M   , class N   , class O   , class P   , class Q               ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p, const Q& q            ),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m,          n,          o,          p,          q            )
    )
    utl_log_member_alias( // 18
        utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                     class J   , class K   , class L   , class M   , class N   , class O   , class P   , class Q   , class R   ),
        utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                     const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p, const Q& q, const R& r),
        utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                              j,          k,          l,          m,          n,          o,          p,          q,          r)
    )
    // clang-format on
};

// =============================
// --- Pre-configured logger ---
// =============================

inline auto& default_logger() {
    static auto logger = Logger{Sink{std::cout}, Sink{"latest.log"}};
    return logger;
}

// Expose default logger err() / warn() / note() / info() / debug() / trace() as functions in the global namespace
// clang-format off
utl_log_function_alias( // 1
    utl_log_hold(class A   ),
    utl_log_hold(const A& a),
    utl_log_hold(         a)
)
utl_log_function_alias( // 2
    utl_log_hold(class A   , class B   ),
    utl_log_hold(const A& a, const B& b),
    utl_log_hold(         a,          b)
)
utl_log_function_alias( // 3
    utl_log_hold(class A   , class B   , class C   ),
    utl_log_hold(const A& a, const B& b, const C& c),
    utl_log_hold(         a,          b,          c)
)
utl_log_function_alias( // 4
    utl_log_hold(class A   , class B   , class C   , class D   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d),
    utl_log_hold(         a,          b,          c,          d)
)
utl_log_function_alias( // 5
    utl_log_hold(class A   , class B   , class C   , class D   , class E   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e),
    utl_log_hold(         a,          b,          c,          d,          e)
)
utl_log_function_alias( // 6
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f),
    utl_log_hold(         a,          b,          c,          d,          e,          f)
)
utl_log_function_alias( // 7
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g)
)
utl_log_function_alias( // 8
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h)
)
utl_log_function_alias( // 9
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i)
)
utl_log_function_alias( // 10
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J                                                                                                   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j                                                                                                ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j                                                                                                )
)
utl_log_function_alias( // 11
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K                                                                                       ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k                                                                                    ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k                                                                                    )
)
utl_log_function_alias( // 12
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L                                                                           ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l                                                                        ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l                                                                        )
)
utl_log_function_alias( // 13
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M                                                               ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m                                                            ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m                                                            )
)
utl_log_function_alias( // 14
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M   , class N                                                   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m, const N& n                                                ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m,          n                                                )
)
utl_log_function_alias( // 15
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M   , class N   , class O                                       ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m, const N& n, const O& o                                    ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m,          n,          o                                    )
)
utl_log_function_alias( // 16
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M   , class N   , class O   , class P                           ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p                        ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m,          n,          o,          p                        )
)
utl_log_function_alias( // 17
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M   , class N   , class O   , class P   , class Q               ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p, const Q& q            ),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m,          n,          o,          p,          q            )
)
utl_log_function_alias( // 18
    utl_log_hold(class A   , class B   , class C   , class D   , class E   , class F   , class G   , class H   , class I   ,
                 class J   , class K   , class L   , class M   , class N   , class O   , class P   , class Q   , class R   ),
    utl_log_hold(const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i,
                 const J& j, const K& k, const L& l, const M& m, const N& n, const O& o, const P& p, const Q& q, const R& r),
    utl_log_hold(         a,          b,          c,          d,          e,          f,          g,          h,          i,
                          j,          k,          l,          m,          n,          o,          p,          q,          r)
)
    // clang-format on

    // ================
    // --- Printing ---
    // ================

    template <class... Args>
    void stringify_append(std::string& str, const Args&... args) {
    // Format all 'args' into a string using the same buffer abstraction as logging sinks, this doesn't add overhead
    StringBuffer buffer(str);
    (Formatter<Args>{}(buffer, args), ...);
}

template <class... Args>
std::string stringify(const Args&... args) {
    std::string res;
    stringify_append(res, args...);
    return res;
}

template <class... Args>
void print(const Args&... args) {
    // Print all 'args' to console in a thread-safe way with instant flushing
    static std::mutex     mutex;
    const std::lock_guard lock(mutex);

    std::cout << stringify(args...) << std::flush;
}

template <class... Args>
void println(const Args&... args) {
    print(args..., '\n');
}

} // namespace utl::log::impl

// ______________________ PUBLIC API ______________________

namespace utl::log {

using impl::Formatter;

using impl::Logger;
using impl::Sink;

namespace policy = impl::policy;

using impl::general;
using impl::fixed;
using impl::scientific;
using impl::hex;
using impl::base;
using impl::align_left;
using impl::align_center;
using impl::align_right;

namespace color = impl::color;

using impl::err;
using impl::warn;
using impl::note;
using impl::info;
using impl::debug;
using impl::trace;

using impl::stringify_append;
using impl::stringify;
using impl::print;
using impl::println;

} // namespace utl::log

#endif
#endif // module utl::log
