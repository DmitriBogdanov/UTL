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
    using integer_type = strong_type::arithmetic<T, class int_tag>;

    constexpr T l = 4;
    constexpr T r = 3;
    constexpr T z = 0;

    constexpr std::size_t shift = 5;
    
    // Accessing the underlying value
    integer_type val = l;
    CHECK(val.get() == l);
    
    val.get() = r; // mutable access
    CHECK(val.get() == r);

    // Increment
    static_assert(++(integer_type{l}) == integer_type{l + 1});
    static_assert(--(integer_type{l}) == integer_type{l - 1});
    static_assert((integer_type{l})++ == integer_type{l});
    static_assert((integer_type{l})-- == integer_type{l});

    // Unary operators
    static_assert(+integer_type{l} == integer_type{static_cast<T>(+l)});
    static_assert(-integer_type{l} == integer_type{static_cast<T>(strong_type::impl::minus(l))});
    static_assert(~integer_type{l} == integer_type{static_cast<T>(~l)});

    // Additive & bitwise operators
    static_assert((integer_type{l} + integer_type{r}) == integer_type{l + r});
    static_assert((integer_type{l} - integer_type{r}) == integer_type{l - r});
    static_assert((integer_type{l} & integer_type{r}) == integer_type{l & r});
    static_assert((integer_type{l} | integer_type{r}) == integer_type{l | r});
    static_assert((integer_type{l} ^ integer_type{r}) == integer_type{l ^ r});

    // Multiplicative operators
    static_assert((integer_type{l} * r) == integer_type{l * r});
    static_assert((l * integer_type{r}) == integer_type{l * r});
    static_assert((integer_type{l} / r) == integer_type{l / r});
    static_assert((integer_type{l} % r) == integer_type{l % r});

    // Arithmetic & bitwise augmented assignment
    static_assert((integer_type{l} += integer_type{r}) == integer_type{l + r});
    static_assert((integer_type{l} -= integer_type{r}) == integer_type{l - r});
    static_assert((integer_type{l} &= integer_type{r}) == integer_type{l & r});
    static_assert((integer_type{l} |= integer_type{r}) == integer_type{l | r});
    static_assert((integer_type{l} ^= integer_type{r}) == integer_type{l ^ r});

    // Multiplicative augmented assignment
    static_assert((integer_type{l} *= r) == integer_type{l * r});
    static_assert((integer_type{l} /= r) == integer_type{l / r});
    static_assert((integer_type{l} %= r) == integer_type{l % r});

    // Comparison
    static_assert(integer_type{l} > integer_type{z});
    static_assert(integer_type{l} >= integer_type{z});
    static_assert(integer_type{z} < integer_type{r});
    static_assert(integer_type{z} <= integer_type{r});
    static_assert(integer_type{z} == integer_type{z});
    static_assert(integer_type{l} != integer_type{r});

    // Shift operators
    static_assert((integer_type{l} << shift) == integer_type{strong_type::impl::lshift(l, shift)});
    static_assert((integer_type{l} >> shift) == integer_type{strong_type::impl::rshift(l, shift)});

    // Shift augmented assignment
    static_assert((integer_type{l} <<= shift) == integer_type{strong_type::impl::lshift(l, shift)});
    static_assert((integer_type{l} >>= shift) == integer_type{strong_type::impl::rshift(l, shift)});
    
    // Explicit cast
    static_assert(static_cast<T>(integer_type{l}) == l);
}

TEST_CASE_TEMPLATE("Arithmetic / Float", T, //
                   float,                   //
                   double                   //
) {
    using float_type = strong_type::arithmetic<T, class float_tag>;

    constexpr T l = static_cast<T>(4.5);
    constexpr T r = static_cast<T>(3.2);
    constexpr T z = static_cast<T>(0.0);

    // Accessing the underlying value
    float_type val = l;
    CHECK(val.get() == l);
    
    val.get() = r; // mutable access
    CHECK(val.get() == r);

    // Unary operators
    static_assert(+float_type{l} == float_type{+l});
    static_assert(-float_type{l} == float_type{-l});

    // Additive operators
    static_assert((float_type{l} + float_type{r}) == float_type{l + r});
    static_assert((float_type{l} - float_type{r}) == float_type{l - r});

    // Multiplicative operators
    static_assert((float_type{l} * r) == float_type{l * r});
    static_assert((l * float_type{r}) == float_type{l * r});
    static_assert((float_type{l} / r) == float_type{l / r});

    // Arithmetic augmented assignment
    static_assert((float_type{l} += r) == float_type{l + r});
    static_assert((float_type{l} -= r) == float_type{l - r});

    // Multiplicative augmented assignment
    static_assert((float_type{l} *= r) == float_type{l * r});
    static_assert((float_type{l} /= r) == float_type{l / r});

    // Comparison
    static_assert(float_type{l} > float_type{z});
    static_assert(float_type{l} >= float_type{z});
    static_assert(float_type{z} < float_type{r});
    static_assert(float_type{z} <= float_type{r});
    static_assert(float_type{z} == float_type{z});
    static_assert(float_type{l} != float_type{r});
    
    // Explicit cast
    static_assert(static_cast<T>(float_type{l}) == l);
}