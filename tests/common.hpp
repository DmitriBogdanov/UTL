// __________________________________ CONTENTS ___________________________________
//
//    Common utils / includes / namespaces used for testing.
//    Reduces test boilerplate, should not be included anywhere else.
// _______________________________________________________________________________

// ___________________ TEST FRAMEWORK  ____________________

#define DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS // makes 'CHECK_THROWS()' not give warning for discarding [[nodiscard]]
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN   // automatically creates 'main()' that runs tests
#include "tests/thirdparty/doctest.h"

// ____________________ STD INCLUDES  _____________________

#include <filesystem>  //  filesystem::
#include <limits>      // numeric_limits<>::
#include <string>      // string_literals::
#include <string_view> // string_view_literals::
#include <type_traits> // enable_if_t<>, is_floating_point_v<>

// ____________________ IMPLEMENTATION ____________________

// =============
// --- Utils ---
// =============

// --- Throw-checking ---
// ----------------------

// template <class Func>
// [[nodiscard]] bool check_if_throws(Func f) noexcept {
//     bool throws = false;
//     try {
//         f();
//     } catch (...) { throws = true; }
//     return throws;
// }

// --- Shortened numeric limits ---
// --------------------------------

template <class T>
using nl = std::numeric_limits<T>;

template <class T>
constexpr auto nlmin = nl<T>::min();

template <class T>
constexpr auto nlmax = nl<T>::max();

// --- Approx. float comparison ---
// --------------------------------

// 'constexpr' approximate float comparison (doctest::Approx only works in runtime)

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class T, require_float<T> = true>
struct Flt {
    T value;
    constexpr Flt(T value) noexcept : value(value) {}
};

template <class T, require_float<T> = true>
[[nodiscard]] constexpr bool operator==(Flt<T> lhs, Flt<T> rhs) noexcept {
    const auto l    = lhs.value;
    const auto r    = rhs.value;
    const auto diff = (l > r) ? (l - r) : (r - l);
    return diff < std::numeric_limits<T>::epsilon();
}
template <class T, require_float<T> = true>
[[nodiscard]] constexpr bool operator==(T lhs, Flt<T> rhs) noexcept {
    return Flt{lhs} == rhs;
}
template <class T, require_float<T> = true>
[[nodiscard]] constexpr bool operator==(Flt<T> lhs, T rhs) noexcept {
    return lhs == Flt{rhs};
}

// --- Macros ---
// --------------

#define FAST_CHECK(arg_)                                                                                               \
    if (!bool(arg_)) CHECK(arg_)
// somehow much faster than doing a raw 'CHECK()' in some cases,
// I assume this is due to the test framework needing to record successful checks

// ==================
// --- Namespaces ---
// ==================

namespace fs = std::filesystem;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace utl {}
using namespace utl;
