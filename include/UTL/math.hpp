// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::math
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_math.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MATH)
#ifndef UTLHEADERGUARD_MATH
#define UTLHEADERGUARD_MATH

#define UTL_MATH_VERSION_MAJOR 0 // [!] module awaiting a rewrite
#define UTL_MATH_VERSION_MINOR 0
#define UTL_MATH_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <algorithm>        // sort(), is_permutation(), reverse()
#include <cassert>          // assert()
#include <cmath>            // cos
#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <tuple>            // get<>(), tuple_size_v<>, apply<>()
#include <type_traits>      // enable_if_t<>, is_floating_point<>, is_arithmetic<>, conditional_t<>, is_integral<>, ...
#include <utility>          // declval<>(), move(), forward<>()
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// With C++20 following functions will be added into the standard:
// - midpoint()

// ____________________ IMPLEMENTATION ____________________

namespace utl::math::impl {

// ======================
// --- SFINAE helpers ---
// ======================

template <bool Cond>
using require = std::enable_if_t<Cond, bool>;

template <class T>
using require_arithmetic = require<std::is_arithmetic_v<T>>;

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class Return, class T, class... Args>
using require_invocable_r = require<std::is_invocable_r_v<Return, T, Args...>>;

template <class T, class... Args>
using require_invocable = require<std::is_invocable_v<T, Args...>>;

// ===================
// --- Type Traits ---
// ===================

#define utl_math_define_trait(trait_name_, ...)                                                                        \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;

utl_math_define_trait(has_size, std::declval<T>().size());
utl_math_define_trait(has_capacity, std::declval<T>().capacity());
utl_math_define_trait(has_data, std::declval<T>().data());
utl_math_define_trait(has_value_type, std::declval<typename T::value_type>());
utl_math_define_trait(has_node_type, std::declval<typename T::node_type>());
utl_math_define_trait(has_allocator_type, std::declval<typename T::allocator_type>());
utl_math_define_trait(has_tuple_size, std::tuple_size<T>::value);
utl_math_define_trait(has_get, std::get<0>(std::declval<T>()));

#undef utl_math_define_trait

// =================
// --- Constants ---
// =================

namespace constants {

constexpr double pi      = 3.14159265358979323846;
constexpr double two_pi  = 2. * pi;
constexpr double half_pi = 0.5 * pi;
constexpr double e       = 2.71828182845904523536;
constexpr double phi     = 1.6180339887498948482; // golden ration

} // namespace constants

// =======================
// --- Basic functions ---
// =======================

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T abs(T x) noexcept {
    return (x > T(0)) ? x : -x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T sign(T x) noexcept {
    if constexpr (std::is_unsigned_v<T>) return (x > T(0)) ? T(1) : T(0);
    else return (x > T(0)) ? T(1) : (x < T(0)) ? T(-1) : T(0);
} // returns -1 / 0 / 1

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T bsign(T x) noexcept {
    if constexpr (std::is_unsigned_v<T>) return T(1);
    else return (x >= T(0)) ? T(1) : T(-1);
} // returns -1 / 1 (1 gets priority in x == 0)

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T sqr(T x) noexcept {
    return x * x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T cube(T x) noexcept {
    return x * x * x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T inv(T x) noexcept {
    return 1. / x; // integers will be cast to float then rounded
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T pow(T x, std::size_t p) noexcept {
    if (p == 0) return T(1);
    if (p == 1) return x;
    const T half_pow = pow(x, p / 2);
    return (p % 2 == 0) ? half_pow * half_pow : half_pow * half_pow * x;
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T midpoint(T a, T b) noexcept {
    return (a + b) * 0.5; // integers will be cast to float then rounded
}

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T absdiff(T a, T b) noexcept {
    return (a > b) ? (a - b) : (b - a);
}

[[nodiscard]] constexpr int signpow(int p) noexcept { return (p % 2 == 0) ? 1 : -1; }

// ===========================
// --- Indicator functions ---
// ===========================

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T heaviside(T x) noexcept {
    return static_cast<T>(x > T(0));
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T kronecker_delta(T i, T j) noexcept {
    return (i == j) ? T(1) : T(0);
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T levi_civita(T i, T j, T k) noexcept {
    if (i == j || j == k || k == i) return T(0);
    const std::size_t inversions = (i > j) + (i > k) + (j > k);
    return (inversions % 2 == 0) ? T(1) : T(-1);
}

// ===========================
// --- Degrees and radians ---
// ===========================

// Degree <-> radian conversion
template <class T, require_float<T> = true>
[[nodiscard]] constexpr T deg_to_rad(T degrees) noexcept {
    constexpr T factor = T(constants::pi / 180.);
    return degrees * factor;
}

template <class T, require_float<T> = true>
[[nodiscard]] constexpr T rad_to_deg(T radians) noexcept {
    constexpr T factor = T(180. / constants::pi);
    return radians * factor;
}

// ===========================
// --- Sequence operations ---
// ===========================

template <class Idx, class Func, require_invocable<Func, Idx> = true>
[[nodiscard]] constexpr auto sum(Idx low, Idx high, Func&& func) noexcept(noexcept(func(Idx{}))) {
    assert(low <= high);
    std::invoke_result_t<Func, Idx> res = 0;
    for (Idx i = low; i <= high; ++i) res += func(i);
    return res;
}

template <class Idx, class Func, require_invocable<Func, Idx> = true>
[[nodiscard]] constexpr auto prod(Idx low, Idx high, Func&& func) noexcept(noexcept(func(Idx{}))) {
    assert(low <= high);
    std::invoke_result_t<Func, Idx> res = 1;
    for (Idx i = low; i <= high; ++i) res *= func(i);
    return res;
}

// ==================
// --- Indexation ---
// ==================

// Same thing as C++20 ssize()
template <class T, require<has_size_v<T>> = true>
[[nodiscard]] constexpr auto ssize(const T& container) {
    using return_type = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(container.size())>>;
    return static_cast<return_type>(container.size());
}

// Utility used to reverse indexation logic, mostly useful when working with unsigned indices
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T reverse_idx(T idx, T size) noexcept {
    return size - T(1) - idx;
}

// ====================
// --- Permutations ---
// ====================

template <class Array>
bool is_permutation(const Array& array) {
    std::vector<std::size_t> p(array.size()); // Note: "non-allocating range adapter" would fit like a glove here
    for (std::size_t i = 0; i < p.size(); ++i) p[i] = i;

    return std::is_permutation(array.begin(), array.end(), p.begin()); // I'm surprised it exists in the standard
}

template <class Array, class Permutation = std::initializer_list<std::size_t>>
void apply_permutation(Array& array, const Permutation& permutation) {
    Array res(array.size());

    typename Array::size_type emplace_idx = 0;
    for (auto i : permutation) res[emplace_idx++] = std::move(array[i]);
    array = std::move(res);
}

template <class Array, class Cmp = std::less<>>
std::vector<std::size_t> sorting_permutation(const Array& array, Cmp cmp = Cmp()) {
    std::vector<std::size_t> permutation(array.size());
    for (std::size_t i = 0; i < permutation.size(); ++i) permutation[i] = i;

    std::sort(permutation.begin(), permutation.end(),
              [&](const auto& lhs, const auto& rhs) { return cmp(array[lhs], array[rhs]); });

    return permutation;
}

template <class Array, class... SyncedArrays>
void sort_together(Array& array, SyncedArrays&... synced_arrays) {
    // Get permutation that would make the 1st array sorted
    const auto permutation = sorting_permutation(array);

    // Apply permutation to all arrays to "sort them in sync"
    apply_permutation(array, permutation);
    (apply_permutation(synced_arrays, permutation), ...);
}

// ==========================
// --- Branchless ternary ---
// ==========================

template <class T, require_arithmetic<T> = true>
[[nodiscard]] constexpr T ternary_branchless(bool condition, T return_if_true, T return_if_false) noexcept {
    return (condition * return_if_true) + (!condition * return_if_false);
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T ternary_bitselect(bool condition, T return_if_true, T return_if_false) noexcept {
    return (return_if_true & -T(condition)) | (return_if_false & ~(-T(condition)));
}

template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T ternary_bitselect(bool condition, T return_if_true /* returns 0 if false*/) noexcept {
    return return_if_true & -T(condition);
}

// ===============
// --- Meshing ---
// ===============

// Semantic helpers that allow user to directly pass both interval/point counts for grid subdivision,
// without thinking about whether function need +1 or -1 to its argument
struct Points {
    std::size_t count;

    Points() = delete;
    constexpr explicit Points(std::size_t count) noexcept : count(count) {}
};

struct Intervals {
    std::size_t count;

    Intervals() = delete;
    constexpr explicit Intervals(std::size_t count) noexcept : count(count) {}
    constexpr Intervals(Points points) noexcept : count(points.count > 0 ? points.count - 1 : 0) {}
};

// Linear 1D mesh
template <class T, require_float<T> = true>
[[nodiscard]] std::vector<T> linspace(T L1, T L2, Intervals N) {
    assert(L1 < L2);
    assert(N.count >= 1);

    std::vector<T> res(N.count + 1);
    for (std::size_t i = 0; i < res.size(); ++i) res[i] = i * (L2 - L1) / N.count;
    return res;
}

// Chebyshev 1D mesh
template <class T, require_float<T> = true>
[[nodiscard]] std::vector<T> chebspace(T L1, T L2, Intervals N) {
    assert(L1 < L2);
    assert(N.count >= 1);

    std::vector<T> res(N.count + 1);
    res.front() = L2;
    for (std::size_t i = 1; i < res.size() - 1; ++i) {
        const std::size_t n  = res.size() - 2;
        const T           c1 = T(0.5) * (L2 + L1);
        const T           c2 = T(0.5) * (L2 - L1);
        const T           c3 = constants::pi * (2 * i - 1) / (2 * n);
        res[i]               = c1 + c2 * std::cos(c3);
    }
    res.back() = L1;

    std::reverse(res.begin(), res.end()); // standard formula goes from '1' to '-1', we want things sorted
    return res;
}

template <class T, class Func, require_float<T> = true, require_invocable_r<T, Func, T> = true>
[[nodiscard]] T integrate_trapezoidal(Func&& f, T L1, T L2, Intervals N) {
    assert(L1 < L2);
    assert(N.count >= 1);

    const T step = (L2 - L1) / N.count;

    T sum = 0;
    T x   = L1;

    for (std::size_t i = 0; i < N.count; ++i, x += step) sum += f(x) + f(x + step);

    return T(0.5) * sum * step;
}

// ====================
// --- Memory usage ---
// ====================

// Below are a few methods to do inaccurate but nonintrusive estimates of containers
// memory usage, we can't really count anything accurately without intrusively knowing implementation
// or providing a custom allocator that gathers statistics, nor can we distinguish heap usage from
// stack usage, however we can get "good enough" estimates to know if something is relatively "big" or
// "small" which is often quite handy for testing & debugging.
//
// Note 1: Node-based containers can be slightly inaccurate due to specifics of node allocation.
//
// Note 2: Contiguous containers should be accurate.
//
// Note 3:
// Estimates for 'std::list', 'std::queue', 'std::dequeue', 'std::priority_queue' are quite inaccurate since unlike
// node-based containers they don't expose the underlying node type to the user, we can only count actual content.

enum class MemoryUnit { BYTE, KiB, MiB, GiB, TiB, KB, MB, GB, TB };

// Workaround for 'static_assert(false)' making program ill-formed even when placed inside an 'if constexpr' branch
// that never compiles. 'static_assert(_always_false_v<T>)' on the the other hand delays its evaluation and works as
// we would want. This is super-well known, this comment just explains the basics should I have amnesia in the future.
template <MemoryUnit units>
constexpr bool _always_false_mem_v = false;

template <class T, class Func>
constexpr void tuple_for_each(T&& tuple, Func&& func) {
    std::apply([&func](auto&&... args) { (func(std::forward<decltype(args)>(args)), ...); }, std::forward<T>(tuple));
}

template <MemoryUnit units = MemoryUnit::MiB>
[[nodiscard]] constexpr double to_memory_units(std::size_t bytes) noexcept {
    if constexpr (units == MemoryUnit::BYTE) return bytes;
    else if constexpr (units == MemoryUnit::KiB) return bytes / 1024.;
    else if constexpr (units == MemoryUnit::MiB) return bytes / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::GiB) return bytes / 1024. / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::TiB) return bytes / 1024. / 1024. / 1024. / 1024.;
    else if constexpr (units == MemoryUnit::KB) return bytes / 1000.;
    else if constexpr (units == MemoryUnit::MB) return bytes / 1000. / 1000.;
    else if constexpr (units == MemoryUnit::GB) return bytes / 1000. / 1000. / 1000.;
    else if constexpr (units == MemoryUnit::TB) return bytes / 1000. / 1000. / 1000. / 1000.;
    else static_assert(_always_false_mem_v<units>, "Function is a non-exhaustive visitor of enum class {MemoryUnit}.");
}

// Quick memory usage estimate, doesn't iterate containers and doesn't try to expand
// recursively, gives the best guess it can get by just querying the size
template <MemoryUnit units = MemoryUnit::MiB, class T>
[[nodiscard]] constexpr double quick_memory_estimate(const T& value) {
    std::size_t bytes{};

    // Node-based containers with size
    // (like 'std::map', 'std::unordered_map', 'std::set', 'std::unordered_set')
    if constexpr (has_size_v<T> && has_node_type_v<T>) {
        bytes += sizeof(T);
        bytes += value.size() * sizeof(typename T::node_type);
    }
    // Contiguous containers with static size
    // (like 'std::array')
    else if constexpr (has_data_v<T> && has_tuple_size_v<T> && has_value_type_v<T>) {
        // counting 'sizeof(T)' here would likely lead to double-counting the same elements
        bytes += std::tuple_size_v<T> * sizeof(typename T::value_type);
    }
    // Contiguous containers with dynamic capacity
    // (like 'std::vector', 'std::string' and most custom ones)
    else if constexpr (has_data_v<T> && has_capacity_v<T> && has_value_type_v<T>) {
        bytes += sizeof(T);
        bytes += value.capacity() * sizeof(typename T::value_type);
    }
    // Contiguous containers with dynamic size
    // (like 'std::list', 'std::queue', 'std::dequeue', 'std::priority_queue')
    else if constexpr (has_data_v<T> && has_size_v<T> && has_value_type_v<T>) {
        bytes += sizeof(T);
        bytes += value.size() * sizeof(typename T::value_type);
    }
    // Tuple-like types
    // (like 'std::tuple', 'std::pair')
    else if constexpr (has_tuple_size_v<T> && has_get_v<T>) {
        tuple_for_each(value, [&](auto&& e) { bytes += quick_memory_estimate(e); });
    }
    // Non-contiguous sized containers
    // (like 'std::list', 'std::queue', 'std::dequeue', 'std::priority_queue')
    else if constexpr (has_size_v<T> && has_value_type_v<T>) {
        bytes += sizeof(T);
        bytes += value.size() * sizeof(typename T::value_type);
    }
    // Everything else
    else {
        bytes += sizeof(T);
    };

    return to_memory_units<units>(bytes);
}

} // namespace utl::math

// ______________________ PUBLIC API ______________________

namespace utl::math {

namespace constants = impl::constants;

using impl::abs;
using impl::sign;
using impl::bsign;
using impl::sqr;
using impl::cube;
using impl::inv;

using impl::pow;
using impl::midpoint;
using impl::absdiff;

using impl::signpow;

using impl::heaviside;
using impl::kronecker_delta;
using impl::levi_civita;

using impl::deg_to_rad;
using impl::rad_to_deg;

using impl::sum;
using impl::prod;

using impl::ssize;
using impl::reverse_idx;

using impl::is_permutation;
using impl::apply_permutation;
using impl::sorting_permutation;
using impl::sort_together;

using impl::ternary_branchless;
using impl::ternary_bitselect;

using impl::Points;
using impl::Intervals;

using impl::linspace;
using impl::chebspace;

using impl::integrate_trapezoidal;

using impl::MemoryUnit;
using impl::to_memory_units;
using impl::quick_memory_estimate;

} // namespace utl::math

#endif
#endif // module utl::math
