#include "tests/common.hpp"

#include "include/UTL/strong_type.hpp"

// _______________________ INCLUDES _______________________

#include <cstdint> // int8_t, int16_t, int32_t, int64_t, ...

// ____________________ IMPLEMENTATION ____________________


TEST_CASE_TEMPLATE("Arithmetic / Integral", T, //
                   std::int8_t,                //
                   std::int16_t,               //
                   std::int32_t,               //
                   std::int64_t,               //
                   std::uint8_t,               //
                   std::uint16_t,              //
                   std::uint32_t,              //
                   std::uint64_t               //
) {
    using Int = strong_type::Arithmetic<T, class IntTag>;

    constexpr T l = 4;
    constexpr T r = 3;
    constexpr T z = 0;

    constexpr std::size_t shift = 5;
    
    // Accessing the underlying value
    Int val = l;
    CHECK(val.get() == l);
    
    val.get() = r; // mutable access
    CHECK(val.get() == r);

    // Increment
    static_assert(++(Int{l}) == Int{l + 1});
    static_assert(--(Int{l}) == Int{l - 1});
    static_assert((Int{l})++ == Int{l});
    static_assert((Int{l})-- == Int{l});

    // Unary operators
    static_assert(+Int{l} == Int{static_cast<T>(+l)});
    static_assert(-Int{l} == Int{static_cast<T>(-l)});
    static_assert(~Int{l} == Int{static_cast<T>(~l)});

    // Additive & bitwise operators
    static_assert((Int{l} + Int{r}) == Int{l + r});
    static_assert((Int{l} - Int{r}) == Int{l - r});
    static_assert((Int{l} & Int{r}) == Int{l & r});
    static_assert((Int{l} | Int{r}) == Int{l | r});
    static_assert((Int{l} ^ Int{r}) == Int{l ^ r});

    // Multiplicative operators
    static_assert((Int{l} * r) == Int{l * r});
    static_assert((l * Int{r}) == Int{l * r});
    static_assert((Int{l} / r) == Int{l / r});
    static_assert((Int{l} % r) == Int{l % r});

    // Arithmetic & bitwise augmented assignment
    static_assert((Int{l} += Int{r}) == Int{l + r});
    static_assert((Int{l} -= Int{r}) == Int{l - r});
    static_assert((Int{l} &= Int{r}) == Int{l & r});
    static_assert((Int{l} |= Int{r}) == Int{l | r});
    static_assert((Int{l} ^= Int{r}) == Int{l ^ r});

    // Multiplicative augmented assignment
    static_assert((Int{l} *= r) == Int{l * r});
    static_assert((Int{l} /= r) == Int{l / r});
    static_assert((Int{l} %= r) == Int{l % r});

    // Comparison
    static_assert(Int{l} > Int{z});
    static_assert(Int{l} >= Int{z});
    static_assert(Int{z} < Int{r});
    static_assert(Int{z} <= Int{r});
    static_assert(Int{z} == Int{z});
    static_assert(Int{l} != Int{r});

    // Shift operators
    static_assert((Int{l} << shift) == Int{strong_type::impl::lshift(l, shift)});
    static_assert((Int{l} >> shift) == Int{strong_type::impl::rshift(l, shift)});

    // Shift augmented assignment
    static_assert((Int{l} <<= shift) == Int{strong_type::impl::lshift(l, shift)});
    static_assert((Int{l} >>= shift) == Int{strong_type::impl::rshift(l, shift)});
    
    // Explicit cast
    static_assert(static_cast<T>(Int{l}) == l);
}

TEST_CASE_TEMPLATE("Arithmetic / Float", T, //
                   float,                   //
                   double                   //
) {
    using Float = strong_type::Arithmetic<T, class FloatTag>;

    constexpr T l = static_cast<T>(4.5);
    constexpr T r = static_cast<T>(3.2);
    constexpr T z = static_cast<T>(0.0);

    // Accessing the underlying value
    Float val = l;
    CHECK(val.get() == l);
    
    val.get() = r; // mutable access
    CHECK(val.get() == r);

    // Unary operators
    static_assert(+Float{l} == Float{+l});
    static_assert(-Float{l} == Float{-l});

    // Additive operators
    static_assert((Float{l} + Float{r}) == Float{l + r});
    static_assert((Float{l} - Float{r}) == Float{l - r});

    // Multiplicative operators
    static_assert((Float{l} * r) == Float{l * r});
    static_assert((l * Float{r}) == Float{l * r});
    static_assert((Float{l} / r) == Float{l / r});

    // Arithmetic augmented assignment
    static_assert((Float{l} += r) == Float{l + r});
    static_assert((Float{l} -= r) == Float{l - r});

    // Multiplicative augmented assignment
    static_assert((Float{l} *= r) == Float{l * r});
    static_assert((Float{l} /= r) == Float{l / r});

    // Comparison
    static_assert(Float{l} > Float{z});
    static_assert(Float{l} >= Float{z});
    static_assert(Float{z} < Float{r});
    static_assert(Float{z} <= Float{r});
    static_assert(Float{z} == Float{z});
    static_assert(Float{l} != Float{r});
    
    // Explicit cast
    static_assert(static_cast<T>(Float{l}) == l);
}