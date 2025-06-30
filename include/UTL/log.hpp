// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::log
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_log.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_LOG)
#ifndef UTLHEADERGUARD_LOG
#define UTLHEADERGUARD_LOG

#define UTL_LOG_VERSION_MAJOR 0 // [!] module awaiting a rewrite
#define UTL_LOG_VERSION_MINOR 1
#define UTL_LOG_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <array>         // array<>
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock
#include <cstddef>       // size_t
#include <exception>     // exception
#include <fstream>       // ofstream
#include <iostream>      // cout
#include <iterator>      // next()
#include <limits>        // numeric_limits<>
#include <list>          // list<>
#include <mutex>         // lock_guard<>, mutex
#include <ostream>       // ostream
#include <sstream>       // std::ostringstream
#include <stdexcept>     // std::runtime_error
#include <string>        // string
#include <string_view>   // string_view
#include <system_error>  // errc()
#include <thread>        // this_thread::get_id()
#include <tuple>         // tuple_size<>
#include <type_traits>   // is_integral_v<>, is_floating_point_v<>, is_same_v<>, is_convertible_to_v<>
#include <unordered_map> // unordered_map<>
#include <utility>       // forward<>()
#include <variant>       // variant<>

// ____________________ DEVELOPER DOCS ____________________

// Reasonably performant and convenient logger.
//
// The main highlight of this module (and the main performance win relative to 'std::ostream') is a
// generic stringifier class that is both convenient and quite fast at formatting multiple values.
//
// This stringifier can be customized with CRTP to format things in all kinds of specific ways
// and will likely come in handy in some other modules.
//
// The logger implementation itself is actually not that efficient (even though not completely
// naive either), a proper performance-oriented approach would use following things:
//
//    1. More 'constexpr' things to avoid having to constantly check styling conditions at runtime
//
//    2. A separate persistent thread to flush the buffer
//
//       Note: I did try using a stripped down version of 'utl::parallel::ThreadPool' to upload tasks
//             for flushing the buffer, it generally improves performance by ~30%, however I decided it
//             is not worth the added complexity & cpu usage for that little gain
//
//    3. Platform-specific methods to query stuff like time & thread id with less overhead
//
//    4. A centralized formatting & info querying facility so multiple sinks don't have to repeat
//       formatting & querying logic.
//
//       Such facility would have to decide the bare minimum of work possible base on all existing
//       sink options, perform the formatting, and then just distribute string view to actual sinks.
//
//       This drastically complicates the logic and can be rather at odds with point (1) since unlike
//       the individual sinks, I don't see a way of making style checks here constexpr, but in the end
//       that would be a proper solution.

// ____________________ IMPLEMENTATION ____________________

namespace utl::log::impl {

// ======================
// --- Internal utils ---
// ======================

// - SFINAE to select localtime_s() or localtime_r() -
template <class TimeMoment, class TimeType>
auto available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer))) {
    return localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer));
}

template <class TimeMoment, class TimeType>
auto available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment))) {
    return localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment));
}

inline std::size_t get_thread_index(const std::thread::id id) {
    static std::mutex     mutex;
    const std::lock_guard lock(mutex);

    static std::unordered_map<std::thread::id, std::size_t> thread_ids;
    static std::size_t                                      next_id = 0;

    const auto it = thread_ids.find(id);
    if (it == thread_ids.end()) return thread_ids[id] = next_id++;
    return it->second;
    // this map effectively "demangles" platform-specific IDs into human-readable IDs (0, 1, 2, ...)
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
unsigned int integer_digit_count(IntType value) {
    unsigned int digits = (value <= 0) ? 1 : 0;
    // (value <  0) => we add 1 digit because of '-' in front
    // (value == 0) => we add 1 digit for '0' because the loop doesn't account for zero integers
    while (value) {
        value /= 10;
        ++digits;
    }
    return digits;
    // Note: There is probably a faster way of doing it
}

using clock = std::chrono::steady_clock;

inline const clock::time_point program_entry_time_point = clock::now();

// ===================
// --- Stringifier ---
// ===================

// --- Internal type traits ---
// ----------------------------

#define utl_log_define_trait(trait_name_, ...)                                                                         \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

utl_log_define_trait(has_real, std::declval<T>().real());
utl_log_define_trait(has_imag, std::declval<T>().imag());
utl_log_define_trait(has_string, std::declval<T>().string());
utl_log_define_trait(has_begin, std::declval<T>().begin());
utl_log_define_trait(has_end, std::declval<T>().end());
utl_log_define_trait(has_input_it, std::next(std::declval<T>().begin()));
utl_log_define_trait(has_get, std::get<0>(std::declval<T>()));
utl_log_define_trait(has_tuple_size, std::tuple_size<T>::value);
utl_log_define_trait(has_container_type, std::declval<typename std::decay_t<T>::container_type>());
utl_log_define_trait(has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());
utl_log_define_trait(is_pad_left, std::declval<std::decay_t<T>>().is_pad_left);
utl_log_define_trait(is_pad_right, std::declval<std::decay_t<T>>().is_pad_right);
utl_log_define_trait(is_pad, std::declval<std::decay_t<T>>().is_pad);

// Note:
// Trait '_has_input_it' is trickier than it may seem. Just doing '++std::declval<T>().begin()' will work
// most of the time, but there are cases where it returns 'false' for very much iterable types.
//
// """
//    Although the expression '++c.begin()' often compiles, it is not guaranteed to do so: 'c.begin()' is an rvalue
//    expression, and there is no LegacyInputIterator requirement that specifies that increment of an rvalue is
///   guaranteed to work. In particular, when iterators are implemented as pointers or its operator++ is
//    lvalue-ref-qualified, '++c.begin()' does not compile, while 'std::next(c.begin())' does.
// """ (c) https://en.cppreference.com/w/cpp/iterator/next
//
// By checking if 'std::next(c.begin())' compiles we can properly check that iterator satisfies input iterator
// requirements, which means we can use it with operator '++' to iterate over the container. Trying to just
// check for operator '++' would lead to false positives, while checking '++c.begin()' would lead to false
// negatives on containers such as 'std::array'. It would seem that 'std::iterator_traits' is the way to go,
// but since it provides no good way to test if iterator satisfies a category without checking every possible
// tag, it ends up being more verbose that existing solution.

#undef utl_log_define_trait

// --- Internal utils ---
// ----------------------

template <class>
constexpr bool always_false_v = false;

template <class T>
constexpr int log10_ceil(T num) {
    return num < 10 ? 1 : 1 + log10_ceil(num / 10);
}

template <class T>
constexpr int max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, log10_ceil(std::numeric_limits<T>::max_exponent10));
// should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
// see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

template <class T>
constexpr int max_int_digits = 2 + std::numeric_limits<T>::digits10;
// +2 because 'digits10' returns last digit index rather than the number of digits
// (aka 1 less than one would expect) and doesn't account for possible '-'.
// Also note that ints use 'digits10' and not 'max_digits10' which is only valid for floats.

// "Unwrapper" for container adaptors such as 'std::queue', 'std::priority_queue', 'std::stack'
template <class Adaptor>
const auto& underlying_container_cref(const Adaptor& adaptor) {

    struct Hack : private Adaptor {
        static const typename Adaptor::container_type& get_container(const Adaptor& adp) {
            return adp.*&Hack::c;
            // An extremely hacky yet standard-compliant way of accessing protected member
            // of a class without actually creating any instances of derived class.
            //
            // This is essentially 2 operators: '.*' and '&',
            // '.*' takes an object on the left side, and a member pointer on the right side,
            // and resolves the pointed-to member of the given object, this means
            // 'object.*&Class::member' is essentially equivalent to 'object.member',
            // except it reveals a "loophole" in protection semantics that allows us to interpret
            // base class object as if it was derived class.
            //
            // Note that doing seemingly much more logical 'static_cast<Derived&>(object).member'
            // is technically UB, even through most compilers will do the reasonable thing.
        }
    };

    return Hack::get_container(adaptor);
}

// --- Alignment ---
// -----------------

// To align/pad values in a stringifier we can wrap then in thin structs
// that gets some special alignment logic in stringifier formatting selector.

template <class T>
struct PadLeft {
    constexpr PadLeft(const T& val, std::size_t size) : val(val), size(size) {} // this is needed for CTAD
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_left = true;
}; // pads value the left (aka right alignment)

template <class T>
struct PadRight {
    constexpr PadRight(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_right = true;
}; // pads value the right (aka left alignment)

template <class T>
struct Pad {
    constexpr Pad(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad = true;
}; // pads value on both sides (aka center alignment)

constexpr std::string_view indent = "    ";

// --- Stringifier ---
// -------------------

// Generic stringifier with customizable API. Formatting of specific types can be customized by inheriting it
// and overriding specific methods. This is a reference implementation that is likely to be used in other modules.
//
// For example of how to extend stringifier see 'utl::log' documentation.
//
template <class Derived>
struct StringifierBase {
    using self    = StringifierBase;
    using derived = Derived;

    // --- Type-wise methods ---
    // -------------------------

    template <class T>
    static void append_bool(std::string& buffer, const T& value) {
        buffer += value ? "true" : "false";
    }

    template <class T>
    static void append_int(std::string& buffer, const T& value) {
        std::array<char, max_int_digits<T>> str;
        const auto [number_end_ptr, error_code] = std::to_chars(str.data(), str.data() + str.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing an integer.");
        buffer.append(str.data(), number_end_ptr - str.data());
    }

    template <class T>
    static void append_enum(std::string& buffer, const T& value) {
        derived::append_int(buffer, static_cast<std::underlying_type_t<T>>(value));
    }

    template <class T>
    static void append_float(std::string& buffer, const T& value) {
        std::array<char, max_float_digits<T>> str;
        const auto [number_end_ptr, error_code] = std::to_chars(str.data(), str.data() + str.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing a float.");
        buffer.append(str.data(), number_end_ptr - str.data());
    }

    template <class T>
    static void append_complex(std::string& buffer, const T& value) {
        derived::append_float(buffer, value.real());
        buffer += " + ";
        derived::append_float(buffer, value.imag());
        buffer += " i";
    }

    template <class T>
    static void append_path(std::string& buffer, const T& value) {
        buffer += value.string();
    }

    template <class T>
    static void append_string(std::string& buffer, const T& value) {
        buffer += value;
    }

    template <class T>
    static void append_array(std::string& buffer, const T& value) {
        buffer += "{ ";
        if (value.begin() != value.end())
            for (auto it = value.begin();;) {
                derived::append(buffer, *it);
                if (++it == value.end()) break; // prevents trailing comma
                buffer += ", ";
            }
        buffer += " }";
    }

    template <class T>
    static void append_tuple(std::string& buffer, const T& value) {
        self::append_tuple_fwd(buffer, value);
    }

    template <class T>
    static void append_adaptor(std::string& buffer, const T& value) {
        derived::append(buffer, underlying_container_cref(value));
    }

    template <class T>
    static void append_printable(std::string& buffer, const T& value) {
        buffer += (std::ostringstream() << value).str();
    }

    // --- Main API ---
    // ----------------

    template <class T>
    static void append(std::string& buffer, const T& value) {
        self::append_selector(buffer, value);
    }

    template <class... Args>
    static void append(std::string& buffer, const Args&... args) {
        (derived::append(buffer, args), ...);
    }

    template <class... Args>
    [[nodiscard]] static std::string stringify(Args&&... args) {
        std::string buffer;
        derived::append(buffer, std::forward<Args>(args)...);
        return buffer;
    }

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return derived::stringify(std::forward<Args>(args)...);
    } // allows stringifier to be used as a functor

    // --- Helpers ---
    // ---------------
private:
    template <class Tuple, std::size_t... Idx>
    static void append_tuple_impl(std::string& buffer, Tuple value, std::index_sequence<Idx...>) {
        ((Idx == 0 ? "" : buffer += ", ", derived::append(buffer, std::get<Idx>(value))), ...);
        // fold expression '( f(args), ... )' invokes 'f(args)' for all arguments in 'args...'
        // in the same fashion, we can fold over 2 functions by doing '( ( f(args), g(args) ), ... )'
    }

    template <template <class...> class Tuple, class... Args>
    static void append_tuple_fwd(std::string& buffer, const Tuple<Args...>& value) {
        buffer += "< ";
        self::append_tuple_impl(buffer, value, std::index_sequence_for<Args...>{});
        buffer += " >";
    }

    template <class T>
    static void append_selector(std::string& buffer, const T& value) {
        // Left-padded something
        if constexpr (is_pad_left_v<T>) {
            std::string temp;
            self::append_selector(temp, value.val);
            if (temp.size() < value.size) buffer.append(value.size - temp.size(), ' ');
            buffer += temp;
        }
        // Right-padded something
        else if constexpr (is_pad_right_v<T>) {
            const std::size_t old_size = buffer.size();
            self::append_selector(buffer, value.val);
            const std::size_t appended_size = buffer.size() - old_size;
            if (appended_size < value.size) buffer.append(value.size - appended_size, ' ');
            // right-padding is faster than left padding since we don't need a temporary string to get appended size
        }
        // Center-padded something
        else if constexpr (is_pad_v<T>) {
            std::string temp;
            self::append_selector(temp, value.val);
            if (temp.size() < value.size) {
                const std::size_t lpad_size = (value.size - temp.size()) / 2;
                const std::size_t rpad_size = value.size - lpad_size - temp.size();
                buffer.append(lpad_size, ' ');
                buffer += temp;
                buffer.append(rpad_size, ' ');
            } else buffer += temp;
        }
        // Bool
        else if constexpr (std::is_same_v<T, bool>)
            derived::append_bool(buffer, value);
        // Char
        else if constexpr (std::is_same_v<T, char>) derived::append_string(buffer, value);
        // 'std::filesystemp::path'-like
        else if constexpr (has_string_v<T>) derived::append_path(buffer, value);
        // 'std::string_view'-convertible (most strings and string-like types)
        else if constexpr (std::is_convertible_v<T, std::string_view>) derived::append_string(buffer, value);
        // 'std::string'-convertible (some "nastier" string-like types, mainly 'std::path')
        else if constexpr (std::is_convertible_v<T, std::string>) derived::append_string(buffer, std::string(value));
        // Integral
        else if constexpr (std::is_integral_v<T>) derived::append_int(buffer, value);
        // Enum
        else if constexpr (std::is_enum_v<T>) derived::append_enum(buffer, value);
        // Floating-point
        else if constexpr (std::is_floating_point_v<T>) derived::append_float(buffer, value);
        // Complex
        else if constexpr (has_real_v<T> && has_imag_v<T>) derived::append_complex(buffer, value);
        // Array-like
        else if constexpr (has_begin_v<T> && has_end_v<T> && has_input_it_v<T>) derived::append_array(buffer, value);
        // Tuple-like
        else if constexpr (has_get_v<T> && has_tuple_size_v<T>) derived::append_tuple(buffer, value);
        // Container adaptor
        else if constexpr (has_container_type_v<T>) derived::append_adaptor(buffer, value);
        // 'std::ostream' printable
        else if constexpr (has_ostream_insert_v<T>) derived::append_printable(buffer, value);
        // No valid stringification exists
        else static_assert(always_false_v<T>, "No valid stringification exists for the type.");

        // Note: Using if-constexpr chain here allows us to pick and choose priority of different branches,
        // removing any possible ambiguity we could encounter doing things through SFINAE or overloads.
    }
};

// Note:
// The stringifier shines at stringifying & concatenating multiple values into the same buffer.
// Single-value is a specific case which allows all 'buffer += ...' to be replaced with most things being formatted
// straight into a newly created string. We could optimize this, but that would make require an almost full logic
// duplication and make the class cumbersome to extend since instead of a singular 'append_something()' methods there
// would be 2: 'append_something()' and 'construct_something()'. It also doesn't seem to be worth it, the difference
// in performance isn't that significant and we're still faster than most usual approaches to stringification.

// ===============================
// --- Stringifier derivatives ---
// ===============================

// "Default" customization of stringifier, here we can optimize a few things.
//
// The reason we don't include this in the original stringifier is because it's intended to be a customizable
// class that can be extended/optimized/decorated by inheriting it and overriding specific methods. The changes
// made by some optimizations wouldn't be compatible with such philosophy.
//
struct Stringifier : public StringifierBase<Stringifier> {
    using base = StringifierBase<Stringifier>;

    // Optimization overloads
    using base::stringify;

    [[nodiscard]] static std::string stringify(int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long long arg) { return std::to_string(arg); }
    // for individual ints 'std::to_string()' beats 'append_int()' with <charconv> since any reasonable compiler
    // implements it using the same <charconv> routine, but formatted directly into the string upon its creation

    [[nodiscard]] static std::string stringify(std::string&& arg) { return std::move(arg); }
    // no need to do all the appending stuff for individual r-value strings, just forward them as is

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return Stringifier::stringify(std::forward<Args>(args)...);
    }
};

template <class... Args>
void append_stringified(std::string& str, Args&&... args) {
    Stringifier::append(str, std::forward<Args>(args)...);
}

template <class... Args>
[[nodiscard]] std::string stringify(Args&&... args) {
    return Stringifier::stringify(std::forward<Args>(args)...);
}

template <class... Args>
void print(Args&&... args) {
    const auto                        res = Stringifier::stringify(std::forward<Args>(args)...);
    static std::mutex                 mutex;
    const std::lock_guard<std::mutex> lock(mutex);
    std::cout << res << std::flush;
    // print in a thread-safe way and instantly flush every message, this is much slower that buffering
    // (which regular logging methods do), but for generic console output this is a more robust way
}

template <class... Args>
void println(Args&&... args) {
    print(std::forward<Args>(args)..., '\n');
}

// ===============
// --- Options ---
// ===============

enum class Verbosity { ERR = 0, WARN = 1, NOTE = 2, INFO = 3, DEBUG = 4, TRACE = 5 };

enum class OpenMode { REWRITE, APPEND };

enum class Colors { ENABLE, DISABLE };

struct Columns {
    bool datetime = true;
    bool uptime   = true;
    bool thread   = true;
    bool callsite = true;
    bool level    = true;
    bool message  = true;

    // Columns() : datetime(true), uptime(true), thread(true), callsite(true), level(true), message(true) {}
};

struct Callsite {
    std::string_view file;
    int              line;
};

struct MessageMetadata {
    Verbosity verbosity;
};

constexpr bool operator<(Verbosity l, Verbosity r) { return static_cast<int>(l) < static_cast<int>(r); }
constexpr bool operator<=(Verbosity l, Verbosity r) { return static_cast<int>(l) <= static_cast<int>(r); }

// --- Column widths ---
// ---------------------

constexpr unsigned int w_uptime_sec = 4;
constexpr unsigned int w_uptime_ms  = 3;

constexpr std::size_t w_callsite_before_dot = 22;
constexpr std::size_t w_callsite_after_dot  = 4;

constexpr std::size_t col_w_datetime = sizeof("yyyy-mm-dd HH:MM:SS") - 1;
constexpr std::size_t col_w_uptime   = w_uptime_sec + 1 + w_uptime_ms;
constexpr std::size_t col_w_thread   = sizeof("thread") - 1;
constexpr std::size_t col_w_callsite = w_callsite_before_dot + 1 + w_callsite_after_dot;
constexpr std::size_t col_w_level    = sizeof("level") - 1;

// --- Column left/right delimiters ---
// ------------------------------------

constexpr std::string_view col_ld_datetime = "";
constexpr std::string_view col_rd_datetime = " ";
constexpr std::string_view col_ld_uptime   = "(";
constexpr std::string_view col_rd_uptime   = ")";
constexpr std::string_view col_ld_thread   = "[";
constexpr std::string_view col_rd_thread   = "]";
constexpr std::string_view col_ld_callsite = " ";
constexpr std::string_view col_rd_callsite = " ";
constexpr std::string_view col_ld_level    = "";
constexpr std::string_view col_rd_level    = "|";
constexpr std::string_view col_ld_message  = " ";
constexpr std::string_view col_rd_message  = "\n";

// --- ANSI Colors ---
// -------------------

namespace color {

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

// logger itself only uses a few of those colors, but since we do it this way might as well provide
// the whole spectrum so users can color 'cout' and 'log::println()' statements too

} // namespace color

constexpr std::string_view color_heading = color::bold_cyan;
constexpr std::string_view color_reset   = color::reset;

constexpr std::string_view color_trace = color::bright_black;
constexpr std::string_view color_debug = color::green;
constexpr std::string_view color_note  = color::magenta;
constexpr std::string_view color_info  = color::white;
constexpr std::string_view color_warn  = color::yellow;
constexpr std::string_view color_err   = color::bold_red;

// ==================
// --- Sink class ---
// ==================

class Sink {
private:
    using os_ref_wrapper = std::reference_wrapper<std::ostream>;

    std::variant<os_ref_wrapper, std::ofstream> os_variant;
    Verbosity                                   verbosity;
    Colors                                      colors;
    clock::duration                             flush_interval;
    Columns                                     columns;
    clock::time_point                           last_flushed;
    bool                                        print_header = true;
    mutable std::mutex                          ostream_mutex;

    friend struct Logger;

    std::ostream& ostream_ref() {
        if (const auto ref_wrapper_ptr = std::get_if<os_ref_wrapper>(&this->os_variant)) return ref_wrapper_ptr->get();
        else return std::get<std::ofstream>(this->os_variant);
    }

public:
    Sink()            = delete;
    Sink(const Sink&) = delete;
    Sink(Sink&&)      = delete;

    Sink(std::ofstream&& os, Verbosity verbosity, Colors colors, clock::duration flush_interval, const Columns& columns)
        : os_variant(std::move(os)), verbosity(verbosity), colors(colors), flush_interval(flush_interval),
          columns(columns) {}

    Sink(std::reference_wrapper<std::ostream> os, Verbosity verbosity, Colors colors, clock::duration flush_interval,
         const Columns& columns)
        : os_variant(os), verbosity(verbosity), colors(colors), flush_interval(flush_interval), columns(columns) {}

    // We want a way of changing sink options using its handle / reference returned by the logger
    Sink& set_verbosity(Verbosity verbosity) {
        this->verbosity = verbosity;
        return *this;
    }
    Sink& set_colors(Colors colors) {
        this->colors = colors;
        return *this;
    }
    Sink& set_flush_interval(clock::duration flush_interval) {
        this->flush_interval = flush_interval;
        return *this;
    }
    Sink& set_columns(const Columns& columns) {
        this->columns = columns;
        return *this;
    }
    Sink& skip_header(bool skip = true) {
        this->print_header = !skip;
        return *this;
    }

private:
    template <class... Args>
    void format(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        if (meta.verbosity > this->verbosity) return;

        thread_local std::string buffer;

        const clock::time_point now = clock::now();

        // To minimize logging overhead we use string buffer, append characters to it and then write the whole buffer
        // to `std::ostream`. This avoids the inherent overhead of ostream formatting (caused largely by
        // virtualization, syncronization and locale handling, neither of which are relevant for the logger).
        //
        // This buffer gets reused between calls. Note the 'thread_local', if buffer was just a class member, multiple
        // threads could fight trying to clear and resize the buffer while it's being written to by another thread.
        //
        // Buffer may grow when formatting a message longer that any one that was formatted before, otherwise we just
        // reuse the reserved memory and no new allocations take place.

        buffer.clear();

        // Print log header on the first call
        {
            static std::mutex     header_mutex;
            const std::lock_guard lock(header_mutex);
            if (this->print_header) {
                this->print_header = false;
                this->format_header(buffer);
            }
            // no need to lock the buffer, other threads can't reach buffer
            // output code while they're stuck waiting for the header to print
        }

        // Format columns one-by-one
        if (this->colors == Colors::ENABLE) switch (meta.verbosity) {
            case Verbosity::ERR: buffer += color_err; break;
            case Verbosity::WARN: buffer += color_warn; break;
            case Verbosity::NOTE: buffer += color_note; break;
            case Verbosity::INFO: buffer += color_info; break;
            case Verbosity::DEBUG: buffer += color_debug; break;
            case Verbosity::TRACE: buffer += color_trace; break;
            }

        if (this->columns.datetime) this->format_column_datetime(buffer);
        if (this->columns.uptime) this->format_column_uptime(buffer, now);
        if (this->columns.thread) this->format_column_thread(buffer);
        if (this->columns.callsite) this->format_column_callsite(buffer, callsite);
        if (this->columns.level) this->format_column_level(buffer, meta.verbosity);
        if (this->columns.message) this->format_column_message(buffer, args...);

        if (this->colors == Colors::ENABLE) buffer += color_reset;

        // 'std::ostream' isn't guaranteed to be thread-safe, even through many implementations seem to have
        // some thread-safety built into `std::cout` the same cannot be said about a generic 'std::ostream'
        const std::lock_guard ostream_lock(this->ostream_mutex);

        this->ostream_ref().write(buffer.data(), buffer.size());

        // flush every message immediately
        if (this->flush_interval.count() == 0) {
            this->ostream_ref().flush();
        }
        // or flush periodically
        else if (now - this->last_flushed > this->flush_interval) {
            this->last_flushed = now;
            this->ostream_ref().flush();
        }
    }

    void format_header(std::string& buffer) {
        if (this->colors == Colors::ENABLE) buffer += color_heading;
        if (this->columns.datetime)
            append_stringified(buffer, col_ld_datetime, PadRight{"date       time", col_w_datetime}, col_rd_datetime);
        if (this->columns.uptime)
            append_stringified(buffer, col_ld_uptime, PadRight{"uptime", col_w_uptime}, col_rd_uptime);
        if (this->columns.thread)
            append_stringified(buffer, col_ld_thread, PadRight{"thread", col_w_thread}, col_rd_thread);
        if (this->columns.callsite)
            append_stringified(buffer, col_ld_callsite, PadRight{"callsite", col_w_callsite}, col_rd_callsite);
        if (this->columns.level) append_stringified(buffer, col_ld_level, PadRight{"level", col_w_level}, col_rd_level);
        if (this->columns.message) append_stringified(buffer, col_ld_message, "message", col_rd_message);
        if (this->colors == Colors::ENABLE) buffer += color_reset;
    }

    void format_column_datetime(std::string& buffer) {
        std::time_t timer = std::time(nullptr);
        std::tm     time_moment{};

        available_localtime_impl(&time_moment, &timer);

        // Format time straight into the buffer
        std::array<char, col_w_datetime + 1> strftime_buffer; // size includes the null terminator added by 'strftime()'
        std::strftime(strftime_buffer.data(), strftime_buffer.size(), "%Y-%m-%d %H:%M:%S", &time_moment);

        // strftime_buffer.back() = ' '; // replace null-terminator added by 'strftime()' with a space
        buffer += col_ld_datetime;
        buffer.append(strftime_buffer.data(), col_w_datetime);
        buffer += col_rd_datetime;
    }

    void format_column_uptime(std::string& buffer, clock::time_point now) {
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - program_entry_time_point);
        const auto sec        = (elapsed_ms / 1000).count();
        const auto ms         = (elapsed_ms % 1000).count(); // is 'elapsed_ms - 1000 * full_seconds; faster?

        const unsigned int sec_digits = integer_digit_count(sec);
        const unsigned int ms_digits  = integer_digit_count(ms);

        buffer += col_ld_uptime;

        // Left-pad the value to column width (doing it manually is a bit faster than using PadLeft{})
        if (sec_digits < w_uptime_sec) buffer.append(w_uptime_sec - sec_digits, ' ');
        append_stringified(buffer, sec);

        buffer += '.';

        // Add leading zeroes to a fixed length
        if (ms_digits < w_uptime_ms) buffer.append(w_uptime_ms - ms_digits, '0');
        append_stringified(buffer, ms);

        buffer += col_rd_uptime;
    }

    void format_column_thread(std::string& buffer) {
        const auto thread_id       = get_thread_index(std::this_thread::get_id());
        const auto thread_id_width = integer_digit_count(thread_id);

        buffer += col_ld_thread;
        if (thread_id_width < col_w_thread) buffer.append(col_w_thread - thread_id_width, ' ');
        append_stringified(buffer, thread_id);
        buffer += col_rd_thread;
    }

    void format_column_callsite(std::string& buffer, const Callsite& callsite) {
        // Get just filename from the full path
        std::string_view filename = callsite.file.substr(callsite.file.find_last_of("/\\") + 1);

        // Left-pad callsite to column width, trim first characters if it's too long
        if (filename.size() < w_callsite_before_dot) buffer.append(w_callsite_before_dot - filename.size(), ' ');
        else filename.remove_prefix(w_callsite_before_dot - filename.size());

        buffer += col_ld_callsite;
        buffer += filename;
        buffer += ':';
        // Right-pad line number
        append_stringified(buffer, callsite.line);
        buffer.append(w_callsite_after_dot - integer_digit_count(callsite.line), ' ');
        buffer += col_rd_callsite;
    }

    void format_column_level(std::string& buffer, Verbosity level) {
        buffer += col_ld_level;
        switch (level) {
        case Verbosity::ERR: buffer += "  ERR"; break;
        case Verbosity::WARN: buffer += " WARN"; break;
        case Verbosity::NOTE: buffer += " NOTE"; break;
        case Verbosity::INFO: buffer += " INFO"; break;
        case Verbosity::DEBUG: buffer += "DEBUG"; break;
        case Verbosity::TRACE: buffer += "TRACE"; break;
        }
        buffer += col_rd_level;
    }

    template <class... Args>
    void format_column_message(std::string& buffer, const Args&... args) {
        buffer += col_ld_message;
        append_stringified(buffer, args...);
        buffer += col_rd_message;
    }
};

// ====================
// --- Logger class ---
// ====================

struct Logger {
    inline static std::list<Sink> sinks;
    // we use list<> because we don't want sinks to ever reallocate when growing / shrinking
    // (reallocation requires a move-constructor, which 'std::mutex' doesn't have),
    // the added overhead of iterating a list is negligible

    static inline Sink default_sink{std::cout, Verbosity::TRACE, Colors::ENABLE, std::chrono::milliseconds(0),
                                    Columns{}};

    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    template <class... Args>
    void push_message(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        // When no sinks were manually created, default sink-to-terminal takes over
        if (this->sinks.empty()) {
            // static Sink default_sink(std::cout, Verbosity::TRACE, Colors::ENABLE, ms(0), Columns{});
            default_sink.format(callsite, meta, args...);
        } else
            for (auto& sink : this->sinks) sink.format(callsite, meta, args...);
    }
};

// =======================
// --- Sink public API ---
// =======================

inline Sink& add_ostream_sink(std::ostream&   os,                                           //
                              Verbosity       verbosity      = Verbosity::INFO,             //
                              Colors          colors         = Colors::ENABLE,              //
                              clock::duration flush_interval = std::chrono::milliseconds{}, //
                              const Columns&  columns        = Columns{}                    //
) {
    return Logger::instance().sinks.emplace_back(os, verbosity, colors, flush_interval, columns);
}

inline Sink& add_file_sink(const std::string& filename,                                       //
                           OpenMode           open_mode      = OpenMode::REWRITE,             //
                           Verbosity          verbosity      = Verbosity::TRACE,              //
                           Colors             colors         = Colors::DISABLE,               //
                           clock::duration    flush_interval = std::chrono::milliseconds{15}, //
                           const Columns&     columns        = Columns{}                      //
) {
    const auto ios_open_mode = (open_mode == OpenMode::APPEND) ? std::ios::out | std::ios::app : std::ios::out;
    return Logger::instance().sinks.emplace_back(std::ofstream(filename, ios_open_mode), verbosity, colors,
                                                 flush_interval, columns);
}

// ======================
// --- Logging macros ---
// ======================

#define UTL_LOG_ERR(...)                                                                                               \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::ERR}, __VA_ARGS__)

#define UTL_LOG_WARN(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::WARN},           \
                                                    __VA_ARGS__)

#define UTL_LOG_NOTE(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::NOTE},           \
                                                    __VA_ARGS__)

#define UTL_LOG_INFO(...)                                                                                              \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::INFO},           \
                                                    __VA_ARGS__)

#define UTL_LOG_DEBUG(...)                                                                                             \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::DEBUG},          \
                                                    __VA_ARGS__)

#define UTL_LOG_TRACE(...)                                                                                             \
    utl::log::impl::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::impl::Verbosity::TRACE},          \
                                                    __VA_ARGS__)

#ifdef _DEBUG
#define UTL_LOG_DERR(...) UTL_LOG_ERR(__VA_ARGS__)
#define UTL_LOG_DWARN(...) UTL_LOG_WARN(__VA_ARGS__)
#define UTL_LOG_DNOTE(...) UTL_LOG_NOTE(__VA_ARGS__)
#define UTL_LOG_DINFO(...) UTL_LOG_INFO(__VA_ARGS__)
#define UTL_LOG_DDEBUG(...) UTL_LOG_DEBUG(__VA_ARGS__)
#define UTL_LOG_DTRACE(...) UTL_LOG_TRACE(__VA_ARGS__)
#else
#define UTL_LOG_DERR(...)
#define UTL_LOG_DWARN(...)
#define UTL_LOG_DNOTE(...)
#define UTL_LOG_DINFO(...)
#define UTL_LOG_DDEBUG(...)
#define UTL_LOG_DTRACE(...)
#endif

} // namespace utl::log::impl

// ______________________ PUBLIC API ______________________

namespace utl::log {

using impl::PadLeft;
using impl::PadRight;
using impl::Pad;

using impl::StringifierBase;
using impl::Stringifier;

using impl::append_stringified;
using impl::stringify;

using impl::print;
using impl::println;

using impl::Verbosity;
using impl::OpenMode;
using impl::Colors;

using impl::Columns;

using impl::Sink;

using impl::add_ostream_sink;
using impl::add_file_sink;

// macro -> ...

} // namespace utl::log

#endif
#endif // module utl::log
