// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::strong_type
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_strong_type.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_STRONG_TYPE)

#ifndef utl_strong_type_headerguard
#define utl_strong_type_headerguard

#define UTL_STRONG_TYPE_VERSION_MAJOR 1
#define UTL_STRONG_TYPE_VERSION_MINOR 0
#define UTL_STRONG_TYPE_VERSION_PATCH 3

// _______________________ INCLUDES _______________________

#include <cassert>     // assert()
#include <type_traits> // enable_if_t<>, is_integral<>, is_floating_point<>, make_unsigned<>, ...
#include <utility>     // size_t, move(), swap()

// ____________________ DEVELOPER DOCS ____________________

// A simple strong type library.
//
// It doesn't provide as many customization points as some other strong type libs do, but it allows
// for a much simpler implementation with significantly less compile time impact since the classic
// trait-per-enabled-operator dispatch requires a great deal of nested template instantiations.
//
// In most actual use cases we just want a simple strongly typed arithmetic value or some kind of immutable
// ID / handle, which makes that complexity go to waste, which why the minimalistic design was chosen.
//
// It also focuses on making things as 'constexpr'-friendly as possible and takes some inspiration
// from 'std::experimental::unique_resource' suggested by paper N4189 back in the day. This part of
// the functionality can be somewhat emulated by exploiting that 'std::unique_ptr<>' works with any
// 'named requirements: NullablePointer' type and can in fact manage a non-pointer data, but doing
// things this way would add a heavy include a prevent a lot of potential 'constexpr'.

// ____________________ IMPLEMENTATION ____________________

namespace utl::strong_type::impl {

// ===============
// --- Utility ---
// ===============

// Binds specific function to a type, useful for wrapping 'C' API functions for a deleter
//
// Note: Explicit SFINAE restrictions on 'operator()' are necessary to make this class work with a 'false'
//       case of 'std::is_invocable<>', without restrictions the trait deduces variadic to be invocable
//       and falls into the ill-formed function body which is a compile error
template <auto function>
struct Bind {
    template <class... Args, std::enable_if_t<std::is_invocable_v<decltype(function), Args...>, bool> = true>
    constexpr auto operator()(Args&&... args) const noexcept(noexcept(function(std::forward<Args>(args)...))) {
        return function(std::forward<Args>(args)...);
    }
};

// =============================
// --- Strongly typed unique ---
// =============================

// --- General case ---
// --------------------

// Strongly typed move-only wrapper around 'T' with a custom deleter.
//
// Useful for wrapping handles returned by 'C' APIs into a strongly typed RAII object.
//
// Examples: OpenGL buffer / shader / program handles.

template <class T, class Tag, class Deleter = void>
class Unique {
    T    value; // potentially default constructible, but not necessarily
    bool active = false;

    static_assert(std::is_move_constructible_v<T>, "Type must be move-constructible.");
    static_assert(std::is_move_assignable_v<T>, "Type must be move-assignable.");
    static_assert(std::is_class_v<Deleter>, "Deleter must be a class.");
    static_assert(std::is_empty_v<Deleter>, "Deleter must be stateless.");
    static_assert(std::is_invocable_v<Deleter, T> || std::is_invocable_v<Deleter, T&>,
                  "Deleter must be invocable for the type.");

    static constexpr bool nothrow_movable =
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>;

public:
    using value_type   = T;
    using tag_type     = Tag;
    using deleter_type = Deleter;

    // Move-only semantics
    Unique(const Unique&)            = delete;
    Unique& operator=(const Unique&) = delete;

    constexpr Unique(Unique&& other) noexcept(nothrow_movable) { *this = std::move(other); }

    constexpr Unique& operator=(Unique&& other) noexcept(nothrow_movable) {
        std::swap(this->value, other.value);
        std::swap(this->active, other.active);

        return *this;
    }

    // Conversion
    constexpr Unique(T&& new_value) noexcept(nothrow_movable) { *this = std::move(new_value); }

    constexpr Unique& operator=(T&& new_value) noexcept(nothrow_movable) {
        this->~Unique(); // don't forget to cleanup existing value

        this->value  = std::move(new_value);
        this->active = true;

        return *this;
    }

    // Accessing the underlying value
    constexpr const T& get() const noexcept { return this->value; }
    constexpr T&       get() noexcept { return this->value; }

    ~Unique() {
        if (this->active) {
            // In MSVC without '/permissive-' or '/Zc:referenceBinding' (which is a subset of '/permissive-')
            // 'std::is_invocable_v<Deleter, T&&>' might be evaluated as 'true' even for deleters that should
            // only accept l-values. This is a warning at C4239 '/W4', but it doesn't affect the behavior and
            // there is literally nothing we can do to work around MSVC blatantly lying about type trait always
            // being 'true' since even instantiating the type trait produces this warning.
#ifdef _MSC_VER
#pragma warning(suppress : 4239, justification : "Non-conformant behavior of MSVC, false positive at '/W4'.")
#endif
            if constexpr (std::is_invocable_v<Deleter, T&&>) deleter_type{}(std::move(this->value));
            else deleter_type{}(this->value); // some APIs take arguments only as an l-value
        }
    }
};

// --- Trivial deleter case ---
// ----------------------------

// Strongly typed move-only wrapper around 'T' with a trivial deleter.
//
// Useful for cases where we don't want (or need) to provide a
// custom deleter, but still want strongly typed move-only semantics.
//
// Trivial deleter case requires a separate specialization with some code duplication because
//    1) Before C++20 only trivial destructors can be 'constexpr'
//    2) There is no way to conditionally compile a destructor
// which means using a trivial 'DefaultDestructor<T>' won't work without sacrificing 'constexpr'.

template <class T, class Tag>
class Unique<T, Tag, void> {
    T value;

    static_assert(std::is_move_constructible_v<T>, "Type must be move-constructible.");
    static_assert(std::is_move_assignable_v<T>, "Type must be move-assignable.");

    static constexpr bool nothrow_movable = std::is_nothrow_move_assignable_v<T>;

public:
    using value_type   = T;
    using tag_type     = Tag;
    using deleter_type = void;

    // Move-only semantics
    Unique(const Unique&)            = delete;
    Unique& operator=(const Unique&) = delete;

    constexpr Unique(Unique&&)            = default;
    constexpr Unique& operator=(Unique&&) = default;

    // Conversion
    constexpr Unique(T&& other) noexcept(nothrow_movable) { *this = std::move(other); }

    constexpr Unique& operator=(T&& other) noexcept(nothrow_movable) {
        this->value = std::move(other);

        return *this;
    }

    // Accessing the underlying value
    constexpr const T& get() const noexcept { return this->value; }
    constexpr T&       get() noexcept { return this->value; }
};

// =============================
// --- No UB signed bitshift ---
// =============================

// Ensure target is two's complement, this includes pretty much every platform ever
// to the point that C++20 standardizes two's complement encoding as a requirement,
// this check exists purely to be pedantic and document our assumptions strictly

static_assert((-1 & 3) == 3);

// before C++20 following options could technically be the case:
//    1. (-1 & 3) == 1 => target is sign & magnitude encoded
//    2. (-1 & 3) == 2 => target is one's complement
//    3. (-1 & 3) == 3 => target is two's complement
// other integer encodings are not possible in the standard
//
// Shifting negative numbers is technically considered UB, in practice every compiler implements
// signed bitshift as '(signed)( (unsigned)x << shift )' however they still act as if calling shift
// on a negative 'x < 0' is UB and therefore can never happen which can lead to weirdness with what
// compiler considers to be dead code elimination. This is why we do the casting explicitly and
// use custom 'lshift()' and 'rshift()' to avoid possible UB.
// see https://stackoverflow.com/a/29710927/28607141
//
// Note: Other bitwise operators ('&', '|', '^', '~') do not suffer from the same issue, their invocation
//       for signed types is implementation-defined rather than UB, which in practice means that they do
//       a reasonable consistent thing since we already enforced two's complement representation.

template <class T>
using require_integral = std::enable_if_t<std::is_integral_v<T>, bool>;

constexpr std::size_t byte_size = 8;

// Left shift,
// unlike regular '<<' works properly with negative values, see notes above
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T lshift(T value, std::size_t shift) noexcept {
    assert(shift < sizeof(T) * byte_size);
    return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) << shift);
}

// Right shift,
// unlike regular '>>' works properly with negative values, see notes above
// undefined behavior if 'shift >= bit_sizeof<T>'
template <class T, require_integral<T> = true>
[[nodiscard]] constexpr T rshift(T value, std::size_t shift) noexcept {
    assert(shift < sizeof(T) * byte_size);
    return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) >> shift);
}

// =============================
// --- Unsigned unary minus ----
// =============================

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] constexpr T minus(T value) noexcept {
    if constexpr (std::is_signed_v<T>) return -value;
    else return ~value + T(1);
    // We need unsigned unary minus for a general case, but MSVC with '/W2' warning level produces a warning when using
    // unary minus with an unsigned value, this warning gets elevated to a compilation error by '/sdl' flag, see
    // https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4146
    //
    // This is a case of MSVC not being standard-compliant, as unsigned '-x' is a perfectly defined operation which
    // evaluates to the same thing as '~x + 1u'. To work around such warning we define this function.
}

// ==================================
// --- Strongly typed arithmetic  ---
// ==================================

template <class>
constexpr bool always_false_v = false;

template <class T, class Tag, class = void>
class Arithmetic {
    static_assert(always_false_v<T>, "'T' must be an arithmetic type (integral or floating-point).");
};

// --- Integer specialization ---
// ------------------------------

// Strongly typed wrapper around an integer.
//
// Supports all of its usual operators with a unit-like behavior (aka 'unit + unit => unit', 'unit * scalar => unit').
// Useful for wrapping conceptually different dimensions & offsets.
//
// Examples: Screen width, screen height, element count, size in bytes.

template <class T, class Tag>
class Arithmetic<T, Tag, std::enable_if_t<std::is_integral_v<T>>> {
    T value = T{};

public:
    // clang-format off
    
    using value_type = T;
    using   tag_type = Tag;
    
    // Copyable semantics
    constexpr Arithmetic           (const Arithmetic& ) = default;
    constexpr Arithmetic           (      Arithmetic&&) = default;
    constexpr Arithmetic& operator=(const Arithmetic& ) = default;
    constexpr Arithmetic& operator=(      Arithmetic&&) = default;
    
    // Conversion
    constexpr Arithmetic           (T new_value) noexcept : value(new_value) {}
    constexpr Arithmetic& operator=(T new_value) noexcept { this->value = new_value; return *this; }

    // Accessing the underlying value
    constexpr const T& get() const noexcept { return this->value; }
    constexpr       T& get()       noexcept { return this->value; }
    
    // Increment
    constexpr Arithmetic& operator++(   ) noexcept {                          ++this->value; return *this; }
    constexpr Arithmetic& operator--(   ) noexcept {                          --this->value; return *this; }
    constexpr Arithmetic  operator++(int) noexcept { const auto temp = *this; ++this->value; return  temp; }
    constexpr Arithmetic  operator--(int) noexcept { const auto temp = *this; --this->value; return  temp; }
    
    // Unary operators
    [[nodiscard]] constexpr Arithmetic operator+() const noexcept { return      +this->value ; }
    [[nodiscard]] constexpr Arithmetic operator-() const noexcept { return minus(this->value); }
    [[nodiscard]] constexpr Arithmetic operator~() const noexcept { return      ~this->value ; }
    
    // Additive & bitwise operators
    [[nodiscard]] constexpr Arithmetic operator+(Arithmetic other) const noexcept { return this->value + other.value; }
    [[nodiscard]] constexpr Arithmetic operator-(Arithmetic other) const noexcept { return this->value - other.value; }
    [[nodiscard]] constexpr Arithmetic operator&(Arithmetic other) const noexcept { return this->value & other.value; }
    [[nodiscard]] constexpr Arithmetic operator|(Arithmetic other) const noexcept { return this->value | other.value; }
    [[nodiscard]] constexpr Arithmetic operator^(Arithmetic other) const noexcept { return this->value ^ other.value; }
    
    // Multiplicative operators
    [[nodiscard]] constexpr Arithmetic operator*(T other) const noexcept { return this->value * other; }
    [[nodiscard]] constexpr Arithmetic operator/(T other) const noexcept { return this->value / other; }
    [[nodiscard]] constexpr Arithmetic operator%(T other) const noexcept { return this->value % other; }
    
    // Arithmetic & bitwise augmented assignment
    [[nodiscard]] constexpr Arithmetic& operator+=(Arithmetic other) noexcept { this->value += other.value; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator-=(Arithmetic other) noexcept { this->value -= other.value; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator^=(Arithmetic other) noexcept { this->value ^= other.value; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator|=(Arithmetic other) noexcept { this->value |= other.value; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator&=(Arithmetic other) noexcept { this->value &= other.value; return *this; }
    
    // Multiplicative augmented assignment
    [[nodiscard]] constexpr Arithmetic& operator*=(T other) noexcept { this->value *= other; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator/=(T other) noexcept { this->value /= other; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator%=(T other) noexcept { this->value %= other; return *this; }
    
    // Comparison
    [[nodiscard]] constexpr bool operator< (Arithmetic other) const noexcept { return this->value <  other.value; }
    [[nodiscard]] constexpr bool operator<=(Arithmetic other) const noexcept { return this->value <= other.value; }
    [[nodiscard]] constexpr bool operator> (Arithmetic other) const noexcept { return this->value >  other.value; }
    [[nodiscard]] constexpr bool operator>=(Arithmetic other) const noexcept { return this->value >= other.value; }
    [[nodiscard]] constexpr bool operator==(Arithmetic other) const noexcept { return this->value == other.value; }
    [[nodiscard]] constexpr bool operator!=(Arithmetic other) const noexcept { return this->value != other.value; }
    
    // Shift operators
    [[nodiscard]] constexpr Arithmetic operator<<(std::size_t shift) const noexcept { return lshift(this->value, shift); }
    [[nodiscard]] constexpr Arithmetic operator>>(std::size_t shift) const noexcept { return rshift(this->value, shift); }
    
    // Shift augmented assignment
    [[nodiscard]] constexpr Arithmetic operator<<=(std::size_t shift) noexcept { *this = *this << shift; return *this; }
    [[nodiscard]] constexpr Arithmetic operator>>=(std::size_t shift) noexcept { *this = *this >> shift; return *this; }
    
    // Explicit cast 
    template <class To>
    [[nodiscard]] constexpr explicit operator To() const noexcept { return static_cast<To>(this->value); }

    // clang-format on
};

// --- Float specialization ---
// ----------------------------

// Strongly typed wrapper around a float.
//
// Supports all of its usual operators with a unit-like behavior (aka 'unit + unit => unit', 'unit * scalar => unit').
// Useful for wrapping conceptually different dimensions & offsets.
//
// Examples: Physical width, physical height, velocity.

template <class T, class Tag>
class Arithmetic<T, Tag, std::enable_if_t<std::is_floating_point_v<T>>> {
    T value = T{};

public:
    // clang-format off
    
    using value_type = T;
    using   tag_type = Tag;
    
    // Copyable semantics
    constexpr Arithmetic           (const Arithmetic& ) = default;
    constexpr Arithmetic           (      Arithmetic&&) = default;
    constexpr Arithmetic& operator=(const Arithmetic& ) = default;
    constexpr Arithmetic& operator=(      Arithmetic&&) = default;
    
    // Conversion
    constexpr Arithmetic           (T other) noexcept : value(other) {}
    constexpr Arithmetic& operator=(T other) noexcept { this->value = other; return *this; }

    // Accessing the underlying value
    constexpr const T& get() const noexcept { return this->value; }
    constexpr       T& get()       noexcept { return this->value; }
    
    // Unary operators
    [[nodiscard]] constexpr Arithmetic operator+() const noexcept { return +this->value; }
    [[nodiscard]] constexpr Arithmetic operator-() const noexcept { return -this->value; }
    
    // Additive operators
    [[nodiscard]] constexpr Arithmetic operator+(Arithmetic other) const noexcept { return this->value + other.value; }
    [[nodiscard]] constexpr Arithmetic operator-(Arithmetic other) const noexcept { return this->value - other.value; }
    
    // Multiplicative operators
    [[nodiscard]] constexpr Arithmetic operator*(T other) const noexcept { return this->value * other; }
    [[nodiscard]] constexpr Arithmetic operator/(T other) const noexcept { return this->value / other; }
    
    // Arithmetic augmented assignment
    [[nodiscard]] constexpr Arithmetic& operator+=(Arithmetic other) noexcept { this->value += other.value; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator-=(Arithmetic other) noexcept { this->value -= other.value; return *this; }
    
    // Multiplicative augmented assignment
    [[nodiscard]] constexpr Arithmetic& operator*=(T other) noexcept { this->value *= other; return *this; }
    [[nodiscard]] constexpr Arithmetic& operator/=(T other) noexcept { this->value /= other; return *this; }
    
    // Comparison
    [[nodiscard]] constexpr bool operator< (Arithmetic other) const noexcept { return this->value <  other.value; }
    [[nodiscard]] constexpr bool operator<=(Arithmetic other) const noexcept { return this->value <= other.value; }
    [[nodiscard]] constexpr bool operator> (Arithmetic other) const noexcept { return this->value >  other.value; }
    [[nodiscard]] constexpr bool operator>=(Arithmetic other) const noexcept { return this->value >= other.value; }
    [[nodiscard]] constexpr bool operator==(Arithmetic other) const noexcept { return this->value == other.value; }
    [[nodiscard]] constexpr bool operator!=(Arithmetic other) const noexcept { return this->value != other.value; }
    
    // Explicit cast 
    template <class To>
    [[nodiscard]] constexpr explicit operator To() const noexcept { return static_cast<To>(this->value); }

    // clang-format on
};

// --- Generic operations ---
// --------------------------

// Inverted multiplication order
template <class T, class Tag>
[[nodiscard]] constexpr Arithmetic<T, Tag> operator*(T lhs, Arithmetic<T, Tag> rhs) noexcept {
    return rhs * lhs;
}

// Makes type usable with 'std::swap' (see https://en.cppreference.com/w/cpp/named_req/Swappable.html)
template <class T, class Tag>
constexpr void swap(Arithmetic<T, Tag> lhs, Arithmetic<T, Tag> rhs) noexcept {
    const auto tmp = lhs;

    lhs = rhs;
    rhs = tmp;
}

} // namespace utl::strong_type::impl

// ______________________ PUBLIC API ______________________

namespace utl::strong_type {

using impl::Bind;
using impl::Unique;
using impl::Arithmetic;

} // namespace utl::strong_type

#endif
#endif // module utl::strong_type
