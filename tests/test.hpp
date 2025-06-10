// __________________________________ CONTENTS ___________________________________
//
//    Utils for testing, common includes and 'using namespace' directives.
//    Intended to reduce boilerplate in the test, should not be included
//    anywhere else.
// _______________________________________________________________________________

// ____________________ STD INCLUDES  _____________________

#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

// ____________________ TESTING UTILS  ____________________

namespace fs = std::filesystem;

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace utl {}
using namespace utl;

template <class Func>
bool check_if_throws(Func f) {
    bool throws = false;
    try {
        f();
    } catch (...) { throws = true; }
    return throws;
}

template <class T>
using nl = std::numeric_limits<T>;

template <class T>
constexpr auto nlmin = nl<T>::min();

template <class T>
constexpr auto nlmax = nl<T>::max();

// Contexpr-approximate float comparison
// (doctest::Approx only works in runtime)

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