// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::random
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_random.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_RANDOM)
#ifndef UTLHEADERGUARD_RANDOM
#define UTLHEADERGUARD_RANDOM

#define UTL_RANDOM_VERSION_MAJOR 2
#define UTL_RANDOM_VERSION_MINOR 1
#define UTL_RANDOM_VERSION_PATCH 2

// _______________________ INCLUDES _______________________

#include <array>            // array<>
#include <cassert>          // assert()
#include <chrono>           // high_resolution_clock
#include <cstdint>          // uint8_t, uint16_t, uint32_t, uint64_t
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::digits, numeric_limits<>::min(), numeric_limits<>::max()
#include <mutex>            // mutex, lock_guard<>
#include <random>           // random_device, uniform_..._distribution<>, generate_canonical<>, seed_seq<>
#include <thread>           // thread::id, this_thread::get_id()
#include <type_traits>      // enable_if_t<>, is_integral<>, is_unsigned<>, is_floating_point<>
#include <utility>          // declval<>()
#include <vector>           // vector<>, hash<>

// ____________________ DEVELOPER DOCS ____________________

// Several <random> compatible PRNGs, slightly improved re-implementations of uniform distributions,
// "better" entropy sources and several convenience wrappers for rng.
//
// Everything implemented here should be portable assuming reasonable assumptions (like existence of
// uint32_t, uint64_t, 8-bit bytes, 32-bit floats, 64-bit doubles and etc.) which hold for most platforms.
//
// Optional macros:
// - #define UTL_RANDOM_USE_INTRINSICS // use rdtsc timestamps for entropy

// ____________________ IMPLEMENTATION ____________________

// ==================================
// --- Optional __rdtsc() support ---
// ==================================

#ifdef UTL_RANDOM_USE_INTRINSICS

// x86 RDTSC timestamps make for a good source of entropy
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define utl_random_cpu_counter __rdtsc()

#else

// Fallback onto a constant that changes with each compilation, not good, but better than nothing
#include <string> // string
#define utl_random_cpu_counter std::hash<std::string>{}(std::string(__TIME__))

#endif

namespace utl::random::impl {

// ============================
// --- SFINAE & type traits ---
// ============================

#define utl_random_define_trait(trait_name_, ...)                                                                      \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;                                                            \
                                                                                                                       \
    template <class T>                                                                                                 \
    using require_##trait_name_ = std::enable_if_t<trait_name_<T>::value, bool>


utl_random_define_trait(is_seed_seq,
                        std::declval<T>().generate(std::declval<std::uint32_t*>(), std::declval<std::uint32_t*>()));
// this type trait is necessary to restrict template constructors & seed functions that take 'SeedSeq&& seq',
// otherwise they will get picked instead of regular seeding methods even for integer arguments.
// This is how standard library seems to do it (based on GCC implementation) so we follow their API.

#undef utl_random_define_trait

template <class>
constexpr bool always_false_v = false;

template <bool Cond>
using require = std::enable_if_t<Cond, bool>; // makes SFINAE a bit less cumbersome

template <class T>
using require_integral = require<std::is_integral_v<T>>;

template <class T>
using require_bool = require<std::is_same_v<T, bool>>;

template <class T>
using require_not_bool = require<!std::is_same_v<T, bool>>;

template <class T>
using require_float = require<std::is_floating_point_v<T>>;

template <class T>
using require_uint = require<std::is_integral_v<T> && std::is_unsigned_v<T>>;

// ====================
// --- 128-bit uint ---
// ====================

// We need 128-bit uint for Lemire's uniform integer distribution algorithm,
// some PRNGs can also make use of wide integers

// GCC & clang provide 128-bit integers as compiler extension
#if defined(__SIZEOF_INT128__) && !defined(__wasm__)
using Uint128 = __uint128_t;

// Otherwise fallback onto either MSVC intrinsics or manual emulation
#else

// Emulation of 128-bit unsigned integer tailored specifically for usage in 64-bit Lemire's algorithm,
// this allows us to skip a lot of generic logic since we really only need 3 things:
//
//    1) 'uint128(x) * uint128(y)'       that performs 64x64 -> 128 bit multiplication
//    2) 'static_cast<std::uint64_t>(x)' that returns lower 64 bits
//    3) 'x >> 64'                       that returns upper 64 bits
//
struct Uint128 {
    std::uint64_t low{}, high{};

    constexpr Uint128(std::uint64_t low) noexcept : low(low) {}
    constexpr explicit Uint128(std::uint64_t low, std::uint64_t high) noexcept : low(low), high(high) {}

    [[nodiscard]] constexpr operator std::uint64_t() const noexcept { return this->low; }

    [[nodiscard]] constexpr Uint128 operator*(Uint128 other) const noexcept {
#if defined(UTL_RANDOM_USE_INTRINSICS) && defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
        // Unlike GCC, MSVC also requires 'UTL_RANDOM_USE_INTRINSICS' flag since it also needs '#include <intrin.h>'
        // for 128-bit multiplication, which could be considered a somewhat intrusive thing to include

        std::uint64_t upper = 0;
        std::uint64_t lower = _umul128(this->low, other.low, &upper);

        return Uint128{lower, upper};

#else

        // Compute all of the cross products
        const std::uint64_t lo_lo = (this->low & 0xFFFFFFFF) * (other.low & 0xFFFFFFFF);
        const std::uint64_t hi_lo = (this->low >> 32) * (other.low & 0xFFFFFFFF);
        const std::uint64_t lo_hi = (this->low & 0xFFFFFFFF) * (other.low >> 32);
        const std::uint64_t hi_hi = (this->low >> 32) * (other.low >> 32);

        // Add products together, this will never overflow
        const std::uint64_t cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
        const std::uint64_t upper = (hi_lo >> 32) + (cross >> 32) + hi_hi;
        const std::uint64_t lower = (cross << 32) | (lo_lo & 0xFFFFFFFF);

        return Uint128{lower, upper};

#endif
    }

    [[nodiscard]] constexpr Uint128 operator>>(int) const noexcept { return this->high; }
};

#endif

// clang-format off
template<class T> struct wider { static_assert(always_false_v<T>, "Missing specialization."); };

template<> struct wider<std::uint8_t > { using type = std::uint16_t; };
template<> struct wider<std::uint16_t> { using type = std::uint32_t; };
template<> struct wider<std::uint32_t> { using type = std::uint64_t; };
template<> struct wider<std::uint64_t> { using type = Uint128;       };

template<class T> using wider_t = typename wider<T>::type;
// clang-format on

// ===========================
// --- Bit-twiddling utils ---
// ===========================

template <class T, require_uint<T> = true>
[[nodiscard]] constexpr T uint_minus(T value) noexcept {
    return ~value + T(1);
    // MSVC with '/W2' warning level gives a warning when using unary minus with an unsigned value, this warning
    // gets elevated to a compilation error by '/sdl' flag, see
    // https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4146
    //
    // This is a case of MSVC not being standard-compliant, as unsigned '-x' is a perfectly defined operation which
    // evaluates to the same thing as '~x + 1u'. To work around such warning we define this function
}

// Merging integers into the bits of a larger one
[[nodiscard]] constexpr std::uint64_t merge_uint32_into_uint64(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::uint64_t>(a) | (static_cast<std::uint64_t>(b) << 32);
}

// Helper method to crush large uints to uint32_t,
// inspired by Melissa E. O'Neil's randutils https://gist.github.com/imneme/540829265469e673d045
template <class T, require_integral<T> = true, require<sizeof(T) <= 8> = true>
[[nodiscard]] constexpr std::uint32_t crush_to_uint32(T value) noexcept {
    if constexpr (sizeof(value) <= 4) {
        return std::uint32_t(value);
    } else {
        const std::uint64_t res = static_cast<std::uint64_t>(value) * 0xbc2ad017d719504d;
        return static_cast<std::uint32_t>(res ^ (res >> 32));
    }
}

// Seed sequence helpers
template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
std::uint32_t seed_seq_to_uint32(SeedSeq&& seq) {
    std::array<std::uint32_t, 1> temp;
    seq.generate(temp.begin(), temp.end());
    return temp[0];
}

template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
std::uint64_t seed_seq_to_uint64(SeedSeq&& seq) {
    std::array<std::uint32_t, 2> temp;
    seq.generate(temp.begin(), temp.end());
    return merge_uint32_into_uint64(temp[0], temp[1]);
}

// 'std::rotl()' from C++20, used by many PRNGs,
// have to use long name because platform-specific includes declare '_rotl' as a macro
template <class T, require_uint<T> = true>
[[nodiscard]] constexpr T rotl(T x, int k) noexcept {
    return (x << k) | (x >> (std::numeric_limits<T>::digits - k));
}

// Some generators shouldn't be zero initialized, in a perfect world the user would never
// do that, but in case they happened to do so regardless we can remap 0 to some "weird"
// value that isn't like to intersect with any other seeds generated by the user. Rejecting
// zero seeds completely wouldn't be appropriate for compatibility reasons.
template <class T, std::size_t N, require_uint<T> = true>
[[nodiscard]] constexpr bool is_zero_state(const std::array<T, N>& state) {
    for (const auto& e : state)
        if (e != T(0)) return false;
    return true;
}

template <class ResultType>
[[nodiscard]] constexpr ResultType mix_seed(ResultType seed) {
    std::uint64_t state = (static_cast<std::uint64_t>(seed) + 0x9E3779B97f4A7C15);
    state               = (state ^ (state >> 30)) * 0xBF58476D1CE4E5B9;
    state               = (state ^ (state >> 27)) * 0x94D049BB133111EB;
    return static_cast<ResultType>(state ^ (state >> 31));
    // some of the 16/32-bit PRNGs have bad correlation on the successive seeds, this can  usually be alleviated
    // by adding a single iteration of a "good" PRNG to pre-mix the seed, here we have an iteration of SplitMix64
}

template <class T, require_uint<T> = true>
constexpr T default_seed = std::numeric_limits<T>::max() / 2 + 1;
// an "overall decent" default seed - doesn't have too many zeroes,
// unlikely to accidentally match with a user-defined seed


// =========================
// --- Random Generators ---
// =========================

// Implementation of several "good" PRNGS.
//
// All generators meets uniform random number generator requirements
// (C++17 and below, see https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)
// (C++20 and above, see https://en.cppreference.com/w/cpp/numeric/random/uniform_random_bit_generator)

// Note:
// Here PRNGs take 'SeedSeq' as a forwarding reference 'SeedSeq&&', while standard PRNGS take 'SeedSeq&',
// this is how it should've been done in the standard too, but for some reason they only standardized
// l-value references, perfect forwarding probably just wasn't in use at the time.

namespace generators {

// --- 16-bit PRNGs ---
// --------------------

// Implementation of 16-bit Romu Mono engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       4 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "RomuMono16" is the fastest 16-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 16-bit PRNG, so if speed is needed at all costs, this one provides it.
//
class RomuMono16 {
public:
    using result_type = std::uint16_t;

private:
    std::uint32_t s{}; // notice 32-bit value as a state rather than two 16-bit ints

public:
    constexpr explicit RomuMono16(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuMono16(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s = (seed & 0x1fffffffu) + 1156979152u; // accepts 29 seed-bits

        for (std::size_t i = 0; i < 10; ++i) this->operator()();
        // naively seeded RomuMono produces correlating patterns on the first iterations
        // for successive seeds, we can do a few iterations to escape that
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint32(seq);

        if (this->s == 0) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = this->s >> 16;
        this->s *= 3611795771u;
        this->s = rotl(this->s, 12);
        return result;
    }
};

// --- 32-bit PRNGs ---
// --------------------

// Implementation of 32-bit splitmix adopted from MurmurHash3, based on paper by Guy L. Steele,
// Doug Lea, and Christine H. Flood. 2014. "Fast splittable pseudorandom number generators"
// see http://marc-b-reynolds.github.io/shf/2017/09/27/LPRNS.html
//     https://gee.cs.oswego.edu/dl/papers/oopsla14.pdf
//     https://github.com/umireon/my-random-stuff/blob/e7b17f992955f4dbb02d4016682113b48b2f6ec1/xorshift/splitmix32.c
//
// Performance: Excellent
// Quality:     3/5
// State:       4 bytes
//
// One of the fastest 32-bit generators that requires only a single 'std::uint32_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
// 32-bit version is somewhat lacking in terms of quality estimate data (relative to the widely used
// 64-bit version), however it still seems to be quite decent.
//
class SplitMix32 {
public:
    using result_type = std::uint32_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix32(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit SplitMix32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        this->s = mix_seed(seed);
        // naively seeded SplitMix32 has a horrible correlation between successive seeds, we can mostly alleviate
        // the issue by pre-mixing the seed with a single iteration of a "better" 64-bit algorithm
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint32(seq);
    }

    constexpr result_type operator()() noexcept {
        result_type result = (this->s += 0x9e3779b9);
        result             = (result ^ (result >> 16)) * 0x21f0aaad;
        result             = (result ^ (result >> 15)) * 0x735a2d97;
        return result ^ (result >> 15);
    }
};

// Implementation of Xoshiro128++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       16 bytes
//
// Excellent choice as a general purpose 32-bit PRNG.
// Battle-tested and provides a good statistical quality at an excellent speed.
//
class Xoshiro128PP {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro128PP(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit Xoshiro128PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = rotl(this->s[0] + this->s[3], 7) + this->s[0];
        const result_type t      = s[1] << 9;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = rotl(this->s[3], 11);
        return result;
    }
};

// Implementation of 32-bit Romu Trio engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       12 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "RomuTrio" is the fastest 32-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 32-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuTrio32 {
public:
    using result_type = std::uint32_t;

private:
    std::array<result_type, 3> s{};

public:
    constexpr explicit RomuTrio32(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuTrio32(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix32 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix32 to initialize its state
        this->s[2] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        seq.generate(this->s.begin(), this->s.end());

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type xp = this->s[0], yp = this->s[1], zp = this->s[2];
        this->s[0] = 3323815723u * zp;
        this->s[1] = yp - xp;
        this->s[1] = rotl(this->s[1], 6);
        this->s[2] = zp - yp;
        this->s[2] = rotl(this->s[2], 22);
        return xp;
    }
};

// --- 64-bit PRNGs ---
// --------------------

// Implementation of fixed-increment version of Java 8's SplittableRandom generator SplitMix64,
// see http://dx.doi.org/10.1145/2714064.2660195
//     http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
//     https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
//
// Performance: Excellent
// Quality:     4/5
// State:       8 bytes
//
// One of the fastest generators passing BigCrush that requires only a single 'std::uint64_t' of state,
// making it the smallest state available. Some other PRNGs recommend using it for seeding their state.
//
class SplitMix64 {
public:
    using result_type = std::uint64_t;

private:
    result_type s{};

public:
    constexpr explicit SplitMix64(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit SplitMix64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept { this->s = seed; }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s = seed_seq_to_uint64(seq);
    }

    constexpr result_type operator()() noexcept {
        std::uint64_t result = (this->s += 0x9E3779B97f4A7C15);
        result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
        result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
        return result ^ (result >> 31);
    }
};

// Implementation of Xoshiro256++ suggested by David Blackman and Sebastiano Vigna,
// see https://prng.di.unimi.it/
//     https://prng.di.unimi.it/xoshiro256plusplus.c
//
// Performance: Good
// Quality:     4/5
// State:       32 bytes
//
// Excellent choice as a general purpose PRNG.
// Used by several modern languages as their default.
//
class Xoshiro256PP {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 4> s{};

public:
    constexpr explicit Xoshiro256PP(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit Xoshiro256PP(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    // while zero-state is considered invalid, PRNG can still produce 0 as a result
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Xoshiro family recommends using
        this->s[1] = splitmix(); // splitmix to initialize its state
        this->s[2] = splitmix();
        this->s[3] = splitmix();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = seed_seq_to_uint64(seq); // since seed_seq produces 32-bit ints,
        this->s[1] = seed_seq_to_uint64(seq); // we have to generate multiple and then
        this->s[2] = seed_seq_to_uint64(seq); // join them into std::uint64_t's to
        this->s[3] = seed_seq_to_uint64(seq); // properly initialize the entire state

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type result = rotl(this->s[0] + this->s[3], 23) + this->s[0];
        const result_type t      = this->s[1] << 17;
        this->s[2] ^= this->s[0];
        this->s[3] ^= this->s[1];
        this->s[1] ^= this->s[2];
        this->s[0] ^= this->s[3];
        this->s[2] ^= t;
        this->s[3] = rotl(this->s[3], 45);
        return result;
    }
};

// Implementation of Romu DuoJr engine from paper by "Mark A. Overton",
// see https://www.romu-random.org/
//     https://www.romu-random.org/romupaper.pdf
//
// Performance: Excellent
// Quality:     2/5
// State:       16 bytes
//
// Romu family provides extremely fast non-linear PRNGs, "DuoJr" is the fastest 64-bit option available
// that still provides some resemblance of quality. There has been some concerns over the math used
// in its original paper (see https://news.ycombinator.com/item?id=22447848), however I'd yet to find
// a faster 64-bit PRNG, so if speed is needed at all cost this one provides it.
//
class RomuDuoJr64 {
public:
    using result_type = std::uint64_t;

private:
    std::array<result_type, 2> s{};

public:
    constexpr explicit RomuDuoJr64(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit RomuDuoJr64(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) noexcept {
        SplitMix64 splitmix{seed};
        this->s[0] = splitmix(); // Like Xoshiro, Romu recommends
        this->s[1] = splitmix(); // using SplitMix64 to initialize its state
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        this->s[0] = seed_seq_to_uint64(seq); // seed_seq returns 32-bit ints, we have to generate
        this->s[1] = seed_seq_to_uint64(seq); // multiple to initialize full state of 64-bit values

        // ensure we don't hit an invalid all-zero state
        if (is_zero_state(this->s)) this->seed(default_seed<result_type>);
    }

    constexpr result_type operator()() noexcept {
        const result_type res = this->s[0];
        this->s[0]            = 15241094284759029579u * this->s[1];
        this->s[1]            = this->s[1] - res;
        this->s[1]            = rotl(this->s[1], 27);
        return res;
    }
};

// --- CSPRNGs ---
// ---------------

// Implementation of ChaCha20 CSPRNG conforming to RFC 7539 standard
// see https://datatracker.ietf.org/doc/html/rfc7539
//     https://www.rfc-editor.org/rfc/rfc7539#section-2.4
//     https://en.wikipedia.org/wiki/Salsa20

// Quarter-round operation for ChaCha20 stream cipher
constexpr void quarter_round(std::uint32_t& a, std::uint32_t& b, std::uint32_t& c, std::uint32_t& d) {
    a += b, d ^= a, d = rotl(d, 16);
    c += d, b ^= c, b = rotl(b, 12);
    a += b, d ^= a, d = rotl(d, 8);
    c += d, b ^= c, b = rotl(b, 7);
}

template <std::size_t rounds>
[[nodiscard]] constexpr std::array<std::uint32_t, 16> chacha_rounds(const std::array<std::uint32_t, 16>& input) {
    auto state = input;

    static_assert(rounds % 2 == 0, "ChaCha rounds happen in pairs, total number should be divisible by 2.");

    constexpr std::size_t alternating_round_pairs = rounds / 2;
    // standard number of ChaCha rounds as per RFC 7539 is 20 (ChaCha20 variation),
    // however there is a strong case for ChaCha12 being a more sensible default,
    // at the moment the upper bound of what seems "crackable" is somewhere around 7 rounds,
    // which is why ChaCha8 is also widely used whenever speed is necessary.

    for (std::size_t i = 0; i < alternating_round_pairs; ++i) {
        // Column rounds
        quarter_round(state[0], state[4], state[8], state[12]);
        quarter_round(state[1], state[5], state[9], state[13]);
        quarter_round(state[2], state[6], state[10], state[14]);
        quarter_round(state[3], state[7], state[11], state[15]);

        // Diagonal rounds
        quarter_round(state[0], state[5], state[10], state[15]);
        quarter_round(state[1], state[6], state[11], state[12]);
        quarter_round(state[2], state[7], state[8], state[13]);
        quarter_round(state[3], state[4], state[9], state[14]);
    }

    for (std::size_t i = 0; i < state.size(); ++i) state[i] += input[i];
    return state;
}

template <std::size_t rounds>
class ChaCha {
public:
    using result_type = std::uint32_t;

private:
    // Initial state components
    std::array<result_type, 8> key{};     // 256-bit key
    std::array<result_type, 3> nonce{};   // 96-bit nonce
    std::uint32_t              counter{}; // 32-bit counter

    // Block
    std::array<result_type, 16> block{};    // holds next 16 random numbers
    std::size_t                 position{}; // current position in the block

    constexpr static std::array<result_type, 4> constant = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    // "Magic constants" for ChaCha20 are defined through bit representations of the following char arrays:
    // { "expa", "nd 3", "2-by", "te k" },
    // what we have here is exactly that except written as 'std::uint32_t'

    constexpr void generate_new_block() {
        // Set ChaCha20 initial state as per RFC 7539
        //
        //          [ const   const const const ]
        // State    [ key     key   key   key   ]
        // matrix = [ key     key   key   key   ]
        //          [ counter nonce nonce nonce ]
        //
        const std::array<std::uint32_t, 16> input = {
            this->constant[0], this->constant[1], this->constant[2], this->constant[3], //
            this->key[0],      this->key[1],      this->key[2],      this->key[3],      //
            this->key[4],      this->key[5],      this->key[6],      this->key[7],      //
            this->counter,     this->nonce[0],    this->nonce[1],    this->nonce[2]     //
        };

        // Fill new block
        this->block = chacha_rounds<rounds>(input);
        ++this->counter;
    }

public:
    constexpr explicit ChaCha(result_type seed = default_seed<result_type>) noexcept { this->seed(seed); }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    explicit ChaCha(SeedSeq&& seq) {
        this->seed(seq);
    }

    [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
    [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr void seed(result_type seed) {
        // Use some other PRNG to setup initial state
        SplitMix32 splitmix{seed};
        for (auto& e : this->key) e = splitmix();
        for (auto& e : this->nonce) e = splitmix();
        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    template <class SeedSeq, require_is_seed_seq<SeedSeq> = true>
    void seed(SeedSeq&& seq) {
        // Seed sequence allows user to introduce more entropy into the state

        seq.generate(this->key.begin(), this->key.end());
        seq.generate(this->nonce.begin(), this->nonce.end());

        this->counter  = 0; // counter can be set to any number, but usually 0 or 1 is used
        this->position = 0;

        this->generate_new_block();
    }

    constexpr result_type operator()() noexcept {
        // Generate new block if necessary
        if (this->position >= 16) {
            this->generate_new_block();
            this->position = 0;
        }

        // Get random value from the block and advance position cursor
        return this->block[this->position++];
    }
};

using ChaCha8  = ChaCha<8>;
using ChaCha12 = ChaCha<12>;
using ChaCha20 = ChaCha<20>;


} // namespace generators

// ===============
// --- Entropy ---
// ===============

inline std::seed_seq entropy_seq() {
    // Ensure thread safety of our entropy source, it should generally work fine even without
    // it, but with this we can be sure things never race
    static std::mutex     entropy_mutex;
    const std::lock_guard entropy_guard(entropy_mutex);

    // Hardware entropy (if implemented),
    // some platforms (mainly MinGW) implements random device as a regular PRNG that
    // doesn't change from run to run, this is horrible, but we can somewhat improve
    // things by mixing other sources of entropy. Since hardware entropy is a rather
    // limited resource we only call it once.
    static std::uint32_t          seed_rd = std::random_device{}();
    // after that we just scramble it with a regular PRNG
    static generators::SplitMix32 splitmix{seed_rd};
    seed_rd = splitmix();

    // Time in nanoseconds (on some platforms microseconds)
    const auto seed_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // Heap address (tends to be random each run on most platforms)
    std::vector<std::uint32_t> dummy_vec(1, seed_rd);
    const std::size_t          heap_address_hash = std::hash<std::uint32_t*>{}(dummy_vec.data());

    // Stack address (also tends to be random)
    const std::size_t stack_address_hash = std::hash<decltype(heap_address_hash)*>{}(&heap_address_hash);

    // CPU counter (if available, hashed compilation time otherwise)
    const auto cpu_counter = static_cast<std::uint64_t>(utl_random_cpu_counter);

    // Thread ID (guarantees different seeding for thread-local PRNGs)
    const std::size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());

    // Note:
    // There are other sources of entropy, such as function addresses,
    // but those can be rather "constant" on some platforms

    return {
        seed_rd,                             //
        crush_to_uint32(seed_time),          //
        crush_to_uint32(heap_address_hash),  //
        crush_to_uint32(stack_address_hash), //
        crush_to_uint32(cpu_counter),        //
        crush_to_uint32(thread_id)           //
    };
}

inline std::uint32_t entropy() {
    auto seq = entropy_seq();
    return seed_seq_to_uint32(seq);
    // returns 'std::uint32_t' to mimic the return type of 'std::random_device', if we return uint64_t
    // brace-initializers will complain about narrowing conversion on some generators. If someone want
    // more entropy than that they can always use the whole sequence as a generic solution.
    // Also having one 'random::entropy()' is much nicer than 'random::entropy_32()' & 'random::entropy_64()'.
}

// =====================
// --- Distributions ---
// =====================

// --- Uniform int distribution ---
// --------------------------------

template <class T, class Gen, require_uint<T> = true>
constexpr T uniform_uint_lemire(Gen& gen, T range) noexcept(noexcept(gen())) {
    using W = wider_t<T>;

    W product = W(gen()) * W(range);
    T low     = T(product);
    if (low < range) {
        while (low < uint_minus(range) % range) {
            product = W(gen()) * W(range);
            low     = T(product);
        }
    }
    return product >> std::numeric_limits<T>::digits;
}

// Reimplementation of libc++ 'std::uniform_int_distribution<>' except
// - constexpr
// - const-qualified (relative to distribution parameters)
// - noexcept as long as 'Gen::operator()' is noexcept, which is true for all generators in this module
// - supports 'std::uint8_t', 'std::int8_t', 'char'
// - produces the same sequence on each platform
// Performance is exactly the same a libc++ version of 'std::uniform_int_distribution<>',
// in fact, it is likely to return the exact same sequence for most types
template <class T, class Gen, require_integral<T> = true>
constexpr T generate_uniform_int(Gen& gen, T min, T max) noexcept {
    using result_type    = T;
    using unsigned_type  = std::make_unsigned_t<result_type>;
    using generated_type = typename Gen::result_type;
    using common_type    = std::common_type_t<unsigned_type, generated_type>;

    constexpr common_type prng_min = Gen::min();
    constexpr common_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr common_type prng_range = prng_max - prng_min;
    constexpr common_type type_range = std::numeric_limits<common_type>::max();
    const common_type     range      = common_type(max) - common_type(min);

    common_type res{};

    // PRNG has enough state for the range
    if (prng_range > range) {
        const common_type ext_range = range + 1; // never overflows due to branch condition

        // PRNG is bit-uniform
        // => use Lemire's algorithm, GCC/clang provide 128-bit arithmetics natively, other
        //    compilers use emulation, Lemire with emulated 128-bit ints performs about the
        //    same as Java's "modx1", which is the best algorithm without wide arithmetics
        if constexpr (prng_range == type_range) {
            res = uniform_uint_lemire<common_type>(gen, ext_range);
        }
        // PRNG is non-uniform (usually because 'prng_min' is '1')
        // => fallback onto a 2-division algorithm
        else {
            const common_type scaling = prng_range / ext_range;
            const common_type past    = ext_range * scaling;

            do { res = common_type(gen()) - prng_min; } while (res >= past);
            res /= scaling;
        }
    }
    // PRNG needs several invocations to acquire enough state for the range
    else if (prng_range < range) {
        common_type temp{};
        do {
            constexpr common_type ext_prng_range = (prng_range < type_range) ? prng_range + 1 : type_range;
            temp = ext_prng_range * generate_uniform_int<common_type>(gen, 0, range / ext_prng_range);
            res  = temp + (common_type(gen()) - prng_min);
        } while (res >= range || res < temp);
    } else {
        res = common_type(gen()) - prng_min;
    }

    return static_cast<T>(min + res);

    // Note 1:
    // 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
    // those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
    // true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
    // on any platform (see https://en.cppreference.com/w/cpp/types/integer)
    //
    // This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
    // This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.

    // Note 2:
    // 'ext_prng_range' has a ternary purely to silence a false compiler warning from about division by zero due to
    // 'prng_range + 1' overflowing into '0' when 'prng_range' is equal to 'type_range'. Falling into this runtime
    // branch requires 'prng_range < range <= type_range' making such situation impossible, here we simply clamp the
    // value to 'type_range' so it doesn't overflow and trip the compiler when analyzing constexpr for potential UB.
    
    // Note 3:
    // 'static_cast<T>()' in return is functionally useless, but prevents some false positive warnings on MSVC
}

// 'static_cast<>()' preserves bit pattern of signed/unsigned integers of the same size as long as
// those integers are two-complement (see https://en.wikipedia.org/wiki/Two's_complement), this is
// true for most platforms and is in fact guaranteed for standard fixed-width types like 'uint32_t'
// on any platform (see https://en.cppreference.com/w/cpp/types/integer)
//
// This means signed integer distribution can simply use unsigned algorithm and reinterpret the result internally.
// This would be a bit nicer semantically with C++20 `std::bit_cast<>`, but not ultimately any different.
template <class T = int, require_integral<T> = true>
struct UniformIntDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    };

    constexpr UniformIntDistribution() = default;
    constexpr UniformIntDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformIntDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr T operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return generate_uniform_int<result_type>(gen, this->pars.min, this->pars.max);
    }

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return generate_uniform_int<result_type>(gen, p.min, p.max);
    } // for std-compatibility

    constexpr void                      reset() const noexcept {} // nothing to reset, provided for std-compatibility
    [[nodiscard]] constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void                      params(const param_type& p) noexcept { *this = UniformIntDistribution(p); }
    [[nodiscard]] constexpr result_type a() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type b() const noexcept { return this->pars.max; }
    [[nodiscard]] constexpr result_type min() const noexcept { return this->pars.min; }
    [[nodiscard]] constexpr result_type max() const noexcept { return this->pars.max; }

    constexpr bool operator==(const UniformIntDistribution& other) noexcept {
        return this->a() == other.a() && this->b() == other.b();
    }
    constexpr bool operator!=(const UniformIntDistribution& other) noexcept { return !(*this == other); }

private:
    param_type pars{};
};

// --- Uniform real distribution ---
// ---------------------------------

// Can't really make things work without making some reasonable assumptions about the size of primitive types,
// this should be satisfied for the vast majority of platforms. Esoteric architectures can manually adapt the
// algorithm if that is necessary.
static_assert(std::numeric_limits<double>::digits == 53, "Platform not supported, 'float' is expected to be 32-bit.");
static_assert(std::numeric_limits<float>::digits == 24, "Platform not supported, 'double' is expected to be 64-bit.");

template <class T>
constexpr int bit_width(T value) noexcept {
    int width = 0;
    while (value >>= 1) ++width;
    return width;
}

// Constexpr reimplementation of 'std::generate_canonical<>()'
template <class T, class Gen>
constexpr T generate_canonical_generic(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr int float_bits = std::numeric_limits<float_type>::digits;
    // always produce enough bits of randomness for the whole mantissa

    constexpr generated_type prng_max = Gen::max();
    constexpr generated_type prng_min = Gen::min();

    constexpr generated_type prng_range = prng_max - prng_min;
    constexpr generated_type type_range = std::numeric_limits<generated_type>::max();

    constexpr int prng_bits = (prng_range < type_range) ? bit_width(prng_range + 1) : 1 + bit_width(prng_range);
    // how many full bits of randomness PRNG produces on each invocation, prng_bits == floor(log2(prng_range + 1)),
    // ternary handles the case that would overflow when (prng_range == type_range)

    constexpr int invocations_needed = [&]() {
        int count = 0;
        for (int generated_bits = 0; generated_bits < float_bits; generated_bits += prng_bits) ++count;
        return count;
    }();
    // GCC and MSVC use runtime conversion to floating point and std::ceil() & std::log() to obtain
    // this value, in MSVC for example we have something like this:
    //    > invocations_needed = std::ceil( float_type(float_bits) / std::log2( float_type(prng_range) + 1 ) )
    // which is not constexpr due to math functions, we can do a similar thing much easier by just counting bits
    // generated per each invocation. This returns the same thing for any sane PRNG, except since it only counts
    // "full bits" esoteric ranges such as [1, 3] which technically have 1.5 bits of randomness will be counted
    // as 1 bit of randomness, thus overestimating the invocations a little. In practice this makes 0 difference
    // since its only matters for exceedingly small 'prng_range' and such PRNGs simply don't exist in nature, and
    // even if they are theoretically used they will simply use a few more invocation to produce a proper result

    constexpr float_type prng_float_max   = static_cast<float_type>(prng_max);
    constexpr float_type prng_float_min   = static_cast<float_type>(prng_min);
    constexpr float_type prng_float_range = (prng_float_max - prng_float_min) + float_type(1);

    float_type res    = float_type(0);
    float_type factor = float_type(1);

    for (int i = 0; i < invocations_needed; ++i) {
        res += (static_cast<float_type>(gen()) - static_cast<float_type>(prng_min)) * factor;
        factor *= prng_float_range;
    }
    res /= factor;
    // same algorithm is used by 'std::generate_canonical<>' in GCC/clang as of 2025, MSVC used to do the same
    // before the P0952R2 overhaul (see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p0952r2.html)

    if (res >= float_type(1)) res = float_type(1) - std::numeric_limits<float_type>::epsilon() / float_type(2);
    // GCC patch the fixes occasional generation of '1's, has non-zero effect on performance

    return res;
}

// Wrapper that adds special case optimizations for `_generate_canonical_generic<>()'
template <class T, class Gen>
constexpr T generate_canonical(Gen& gen) noexcept(noexcept(gen())) {
    using float_type     = T;
    using generated_type = typename Gen::result_type;

    constexpr generated_type prng_min = Gen::min();
    constexpr generated_type prng_max = Gen::max();

    static_assert(prng_min < prng_max, "UniformRandomBitGenerator requires 'min() < max()'");

    constexpr generated_type prng_range          = prng_max - prng_min;
    constexpr generated_type type_range          = std::numeric_limits<generated_type>::max();
    constexpr bool           prng_is_bit_uniform = (prng_range == type_range);

    constexpr int exponent_bits_64 = 11;
    constexpr int exponent_bits_32 = 8;

    constexpr double mantissa_hex_64 = 0x1.0p-53;  // == 2^-53, corresponds to 53 significant bits of double
    constexpr float  mantissa_hex_32 = 0x1.0p-24f; // == 2^-24, corresponds to 24 significant bits of float

    constexpr double pow2_minus_64 = 0x1.0p-64; // == 2^-64
    constexpr double pow2_minus_32 = 0x1.0p-32; // == 2^-32

    // Note 1: Note hexadecimal float literals, 'p' separates hex-base from the exponent
    // Note 2: Floats have 'mantissa_size + 1' significant bits due to having a sign bit
    // Note 3: All of the methods below produce [0, 1) range

    // Bit-uniform PRNGs can be simply bitmasked & shifted to obtain mantissa
    // 64-bit float, 64-bit uniform PRNG
    // => multiplication algorithm, see [https://prng.di.unimi.it/]
    if constexpr (prng_is_bit_uniform && sizeof(float_type) == 8 && sizeof(generated_type) == 8) {
        return (gen() >> exponent_bits_64) * mantissa_hex_64;
    }
    // 64-bit float, 32-bit uniform PRNG
    // => "low-high" algorithm, see [https://www.doornik.com/research/randomdouble.pdf]
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 8 && sizeof(generated_type) == 4) {
        return (gen() * pow2_minus_64) + (gen() * pow2_minus_32);
    }
    // 32-bit float, 64-bit uniform PRNG
    // => discard bits + multiplication algorithm
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 8) {
        return (static_cast<std::uint32_t>(gen()) >> exponent_bits_32) * mantissa_hex_32;
    }
    // 32-bit float, 32-bit uniform PRNG
    // => multiplication algorithm tweaked for 32-bit
    else if constexpr (prng_is_bit_uniform && sizeof(T) == 4 && sizeof(generated_type) == 4) {
        return (gen() >> exponent_bits_32) * mantissa_hex_32;
    }
    // Generic case, no particular optimizations can be made
    else {
        return generate_canonical_generic<T>(gen);
    }
}

template <class T = double, require_float<T> = true>
struct UniformRealDistribution {
    using result_type = T;

    struct param_type {
        result_type min = 0;
        result_type max = std::numeric_limits<result_type>::max();
    } pars{};

    constexpr UniformRealDistribution() = default;
    constexpr UniformRealDistribution(T min, T max) noexcept : pars({min, max}) { assert(min < max); }
    constexpr UniformRealDistribution(const param_type& p) noexcept : pars(p) { assert(p.min < p.max); }

    template <class Gen>
    constexpr result_type operator()(Gen& gen) const noexcept(noexcept(gen())) {
        return this->pars.min + generate_canonical<result_type>(gen) * (this->pars.max - this->pars.min);
    }

    template <class Gen>
    constexpr T operator()(Gen& gen, const param_type& p) const noexcept(noexcept(gen())) {
        assert(p.min < p.max);
        return p.min + generate_canonical<result_type>(gen) * (p.max - p.min);
    } // for std-compatibility

    constexpr void        reset() const noexcept {} // nothing to reset, provided for std-API compatibility
    constexpr param_type  params() const noexcept { return this->pars; }
    constexpr void        params(const param_type& p) noexcept { *this = UniformRealDistribution(p); }
    constexpr result_type a() const noexcept { return this->pars.min; }
    constexpr result_type b() const noexcept { return this->pars.max; }
    constexpr result_type min() const noexcept { return this->pars.min; }
    constexpr result_type max() const noexcept { return this->pars.max; }

    constexpr bool operator==(const UniformRealDistribution& other) noexcept {
        return this->a() == other.a() && this->b() == other.b();
    }
    constexpr bool operator!=(const UniformRealDistribution& other) noexcept { return !(*this == other); }
};

// --- Normal distribution ---
// ---------------------------

template <class T = double, require_float<T> = true>
struct NormalDistribution {
    using result_type = T;

    struct param_type {
        result_type mean   = 0;
        result_type stddev = 1;
    } pars{};

private:
    // Marsaglia Polar algorithm generates values in pairs so we need to cache the 2nd one
    result_type saved           = 0;
    bool        saved_available = false;

    // Implementation of Marsaglia Polar method for N(0, 1) based on libstdc++,
    // the algorithm is exactly the same, except we use a faster uniform distribution
    // ('generate_canonical()' that was implemented earlier)
    //
    // Note 1:
    // Even if 'generate_canonical()' produced [0, 1] range instead of [0, 1),
    // this would not be an issue since Marsaglia Polar is a rejection method and does
    // not care about the inclusion of upper-boundaries, they get rejected by 'r2 > T(1)' check
    //
    // Note 2:
    // As far as normal distributions go we have 3 options:
    //    - Box-Muller
    //    - Marsaglia Polar
    //    - Ziggurat
    // Box-Muller performance is similar to Marsaglia Polar, but it has issues working with [0, 1]
    // 'generate_canonical()'. Ziggurat is usually ~50% faster, but involver several KB of lookup tables
    // and a MUCH more cumbersome and difficult to generalize implementation. Most (in fact, all I've seen so far)
    // ziggurat implementations found online are absolutely atrocious. There is a very interesting and well-made
    // paper by Christopher McFarland (2015, see https://pmc.ncbi.nlm.nih.gov/articles/PMC4812161/ for pdf) than
    // proposes several significant improvements, but it has even more lookup tables (~12 KB in total) and an even
    // harder implementation. For the sake of robustness we will stick to Polar method for now.
    //
    // Note 3:
    // Not 'constexpr' due to the <cmath> nonsense, can't do anything about it, will be fixed with C++23.
    //
    template <class Gen>
    result_type generate_standard_normal(Gen& gen) noexcept {
        if (this->saved_available) {
            this->saved_available = false;
            return this->saved;
        }

        result_type x, y, r2;

        do {
            x  = T(2) * generate_canonical<result_type>(gen) - T(1);
            y  = T(2) * generate_canonical<result_type>(gen) - T(1);
            r2 = x * x + y * y;
        } while (r2 > T(1) || r2 == T(0));

        const result_type mult = std::sqrt(-2 * std::log(r2) / r2);

        this->saved_available = true;
        this->saved           = x * mult;

        return y * mult;
    }

public:
    constexpr NormalDistribution() = default;
    constexpr NormalDistribution(T mean, T stddev) noexcept : pars({mean, stddev}) { assert(stddev >= T(0)); }
    constexpr NormalDistribution(const param_type& p) noexcept : pars(p) { assert(p.stddev >= T(0)); }

    template <class Gen>
    result_type operator()(Gen& gen) noexcept {
        return this->generate_standard_normal(gen) * this->pars.stddev + this->pars.mean;
    }

    template <class Gen>
    result_type operator()(Gen& gen, const param_type& params) noexcept {
        assert(params.stddev >= T(0));
        return this->generate_standard_normal(gen) * params.stddev + params.mean;
    }

    constexpr void reset() noexcept {
        this->saved           = 0;
        this->saved_available = false;
    }
    [[nodiscard]] constexpr param_type  param() const noexcept { return this->pars; }
    constexpr void                      param(const param_type& p) noexcept { *this = NormalDistribution(p); }
    [[nodiscard]] constexpr result_type mean() const noexcept { return this->pars.mean; }
    [[nodiscard]] constexpr result_type stddev() const noexcept { return this->pars.stddev; }
    [[nodiscard]] constexpr result_type min() const noexcept { return std::numeric_limits<result_type>::lowest(); }
    [[nodiscard]] constexpr result_type max() const noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr bool operator==(const NormalDistribution& other) noexcept {
        return this->mean() == other.mean() && this->stddev() == other.stddev() &&
               this->saved_available == other.saved_available && this->saved == other.saved;
    }
    constexpr bool operator!=(const NormalDistribution& other) noexcept { return !(*this == other); }
};

// --- Approximate normal distribution ---
// ---------------------------------------

// Extremely fast, but noticeably imprecise normal distribution, can be very useful for fuzzing & gamedev

template <class T, require_uint<T> = true>
[[nodiscard]] constexpr int popcount(T x) noexcept {
    constexpr auto bitmask_1 = T(0x5555555555555555UL);
    constexpr auto bitmask_2 = T(0x3333333333333333UL);
    constexpr auto bitmask_3 = T(0x0F0F0F0F0F0F0F0FUL);

    constexpr auto bitmask_16 = T(0x00FF00FF00FF00FFUL);
    constexpr auto bitmask_32 = T(0x0000FFFF0000FFFFUL);
    constexpr auto bitmask_64 = T(0x00000000FFFFFFFFUL);

    x = (x & bitmask_1) + ((x >> 1) & bitmask_1);
    x = (x & bitmask_2) + ((x >> 2) & bitmask_2);
    x = (x & bitmask_3) + ((x >> 4) & bitmask_3);

    if constexpr (sizeof(T) > 1) x = (x & bitmask_16) + ((x >> 8) & bitmask_16);
    if constexpr (sizeof(T) > 2) x = (x & bitmask_32) + ((x >> 16) & bitmask_32);
    if constexpr (sizeof(T) > 4) x = (x & bitmask_64) + ((x >> 32) & bitmask_64);

    return x; // GCC seem to be smart enough to replace this with a built-in
} // C++20 adds a proper 'std::popcount()'

// Quick approximation of normal distribution based on this excellent reddit thread:
// https://www.reddit.com/r/algorithms/comments/yyz59u/fast_approximate_gaussian_generator/
//
// Lack of <cmath> functions also allows us to 'constexpr' everything

template <class T>
[[nodiscard]] constexpr T approx_standard_normal_from_u32_pair(std::uint32_t major, std::uint32_t minor) noexcept {
    constexpr T delta = T(1) / T(4294967296); // (1 / 2^32)

    T x = T(popcount(major)); // random binomially distributed integer 0 to 32
    x += minor * delta;       // linearly fill the gaps between integers
    x -= T(16.5);             // re-center around 0 (the mean should be 16+0.5)
    x *= T(0.3535534);        // scale to ~1 standard deviation
    return x;

    // 'x' now has a mean of 0, stddev very close to 1, and lies strictly in [-5.833631, 5.833631] range,
    // there are exactly 33 * 2^32 possible outputs which is slightly more than 37 bits of entropy,
    // the distribution is approximated via 33 equally spaced intervals each of which is further subdivided
    // into 2^32 parts. As a result we have a very fast, but noticeably inaccurate approximation, not suitable
    // for research, but might prove very useful in fuzzing / gamedev where quality is not that important.
}

template <class T>
[[nodiscard]] constexpr T approx_standard_normal_from_u64(std::uint64_t rng) noexcept {
    return approx_standard_normal_from_u32_pair<T>(static_cast<std::uint32_t>(rng >> 32),
                                                   static_cast<std::uint32_t>(rng));
}

template <class T, class Gen>
constexpr T approx_standard_normal(Gen& gen) noexcept {
    // Ensure PRNG is bit-uniform
    using generated_type = typename Gen::result_type;

    static_assert(Gen::min() == 0);
    static_assert(Gen::max() == std::numeric_limits<generated_type>::max());

    // Forward PRNG to a fast approximation
    if constexpr (sizeof(generated_type) == 8) {
        return approx_standard_normal_from_u64<T>(gen());
    } else if constexpr (sizeof(generated_type) == 4) {
        return approx_standard_normal_from_u32_pair<T>(gen(), gen());
    } else {
        static_assert(always_false_v<T>, "ApproxNormalDistribution<> only supports bit-uniform 32/64-bit PRNGs.");
        // we could use a slower fallback for esoteric PRNGs, but I think it's better to explicitly state when "fast
        // approximate" is not available, esoteric PRNGs are already handled by a regular NormalDistribution
    }
}

template <class T = double, require_float<T> = true>
struct ApproxNormalDistribution {
    using result_type = T;

    struct param_type {
        result_type mean   = 0;
        result_type stddev = 1;
    } pars{};

    constexpr ApproxNormalDistribution() = default;
    constexpr ApproxNormalDistribution(T mean, T stddev) noexcept : pars({mean, stddev}) { assert(stddev >= T(0)); }
    constexpr ApproxNormalDistribution(const param_type& p) noexcept : pars(p) { assert(p.stddev >= T(0)); }

    template <class Gen>
    constexpr result_type operator()(Gen& gen) const noexcept {
        return approx_standard_normal<result_type>(gen) * this->pars.stddev + this->pars.mean;
    }

    template <class Gen>
    constexpr result_type operator()(Gen& gen, const param_type& params) const noexcept {
        assert(params.stddev >= T(0));
        return approx_standard_normal<result_type>(gen) * params.stddev + params.mean;
    }

    constexpr void                     reset() const noexcept {} // nothing to reset, provided for std-API compatibility
    [[nodiscard]] constexpr param_type param() const noexcept { return this->pars; }
    constexpr void                     param(const param_type& p) noexcept { *this = NormalDistribution(p); }
    [[nodiscard]] constexpr result_type mean() const noexcept { return this->pars.mean; }
    [[nodiscard]] constexpr result_type stddev() const noexcept { return this->pars.stddev; }
    [[nodiscard]] constexpr result_type min() const noexcept { return std::numeric_limits<result_type>::lowest(); }
    [[nodiscard]] constexpr result_type max() const noexcept { return std::numeric_limits<result_type>::max(); }

    constexpr bool operator==(const ApproxNormalDistribution& other) noexcept {
        return this->mean() == other.mean() && this->stddev() == other.stddev();
    }
    constexpr bool operator!=(const ApproxNormalDistribution& other) noexcept { return !(*this == other); }
};

// =========================
// --- Convenient random ---
// =========================

// Note 1:
// Despite the intuitive judgement, creating new distribution objects on each call doesn't introduce
// any meaningful overhead. There is however a bit of additional overhead due to 'thread_local' branch
// caused by the thread local PRNG. It is still much faster that 'rand()' or regular <random> usage, but
// if user want to have the bleeding edge performance they should use distributions manually.

// Note 2:
// No '[[nodiscard]]' since random functions inherently can't be pure due to advancing the generator state.
// Discarding return values while not very sensible, can still be done for the sake of advancing state.

// Note 3:
// "Convenient" random uses a thread-local PRNG lazily initialized with entropy, this is a very sane default for
// most cases. If explicit seeding is needed it can be achieved with 'thread_local_prng().seed(...)'.

// --- Global PRNG ---
// -------------------

using PRNG = generators::Xoshiro256PP;

PRNG& thread_local_prng() {
    // no '[[nodiscard]]' as it can be used for a side effect of initializing PRNG
    // no 'noexcept' because entropy source can allocate & fail
    thread_local PRNG prng(entropy_seq());
    return prng;
}

// --- Distribution functions ---
// ------------------------------

// Generic variate, other functions are implemented in terms of this one
template <class Dist>
auto variate(Dist&& dist) -> typename std::decay_t<Dist>::result_type {
    return dist(thread_local_prng());
}

// Integer U[min, max]
template <class T, require_integral<T> = true>
T uniform(T min, T max) {
    return variate(UniformIntDistribution<T>{min, max});
}

// Boolean U[0, 1]
template <class T, require_integral<T> = true, require_bool<T> = true>
T uniform() {
    return variate(UniformIntDistribution<std::uint8_t>{0, 1});
}

// Float U[min, max)
template <class T, require_float<T> = true>
T uniform(T min, T max) {
    return variate(UniformRealDistribution<T>{min, max});
}

// Float U[0, 1)
template <class T, require_float<T> = true>
T uniform() {
    return variate(UniformRealDistribution<T>{0, 1});
}

// Float N(mean, stddev)
template <class T, require_float<T> = true>
T normal(T mean, T stddev) {
    return variate(NormalDistribution<T>{mean, stddev});
    // slower due to discarding state, but that is unavoidable in this API
}

// Float N(0, 1)
template <class T, require_float<T> = true>
T normal() {
    thread_local NormalDistribution<T> dist{};
    return variate(dist);
    // this version can use distribution state properly due to the constant parameters
}

// Choose random element from a list
template <class T>
T choose(std::initializer_list<T> list) {
    return list.begin()[uniform<std::size_t>(0, list.size() - 1)];
}

template <class Container>
auto choose(const Container& list) {
    return list.at(uniform<std::size_t>(0, list.size() - 1));
}

// --- Typed shortcuts ---
// -----------------------

using Uint = unsigned int;

// clang-format off
inline    int uniform_int   (   int min,    int max) { return uniform<   int>(min, max); }
inline   Uint uniform_uint  (  Uint min,   Uint max) { return uniform<  Uint>(min, max); }
inline   bool uniform_bool  (                      ) { return uniform<  bool>(        ); }
inline  float uniform_float ( float min,  float max) { return uniform< float>(min, max); }
inline double uniform_double(double min, double max) { return uniform<double>(min, max); }
inline  float uniform_float (                      ) { return uniform< float>(        ); }
inline double uniform_double(                      ) { return uniform<double>(        ); }

inline  float normal_float ( float mean,  float stddev) { return  normal< float>(mean, stddev); }
inline double normal_double(double mean, double stddev) { return  normal<double>(mean, stddev); }
inline  float normal_float (                          ) { return  normal< float>(            ); }
inline double normal_double(                          ) { return  normal<double>(            ); }
// clang-format on

int    uniform_int(int min, int max);
Uint   uniform_uint(Uint min, Uint max);
bool   uniform_bool();
float  uniform_float(float min, float max);
double uniform_double(double min, double max);
float  uniform_float();
double uniform_double();

float  normal_float(float mean, float stddev);
double normal_double(double mean, double stddev);
float  normal_float();
double normal_double();

} // namespace utl::random::impl

// ______________________ PUBLIC API ______________________

namespace utl::random {

namespace generators = impl::generators;

using impl::entropy_seq;
using impl::entropy;

using impl::UniformIntDistribution;
using impl::UniformRealDistribution;
using impl::NormalDistribution;
using impl::ApproxNormalDistribution;
using impl::generate_canonical;

using impl::PRNG;
using impl::thread_local_prng;

using impl::choose;
using impl::variate;
using impl::uniform;
using impl::normal;

using impl::Uint;
using impl::uniform_int;
using impl::uniform_uint;
using impl::uniform_bool;
using impl::uniform_float;
using impl::uniform_double;
using impl::normal_float;
using impl::normal_double;

using impl::choose;

} // namespace utl::random

#endif
#endif // module utl::random