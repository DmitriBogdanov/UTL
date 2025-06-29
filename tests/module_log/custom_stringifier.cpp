#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Tests for custom stringification

// Create customization of stringifier that also adds '$' on both sides of the floats.
template <class Derived>
struct DecoratedStringifierBase : public log::StringifierBase<Derived> {
    using base = log::StringifierBase<Derived>;

    template <class T>
    static void append_float(std::string& buffer, const T& value) {
        buffer += '$';
        base::append_float(buffer, value);
        buffer += '$';
    }
};

struct DecoratedStringifier : public DecoratedStringifierBase<DecoratedStringifier> {};

// Create another customization on top of prev. one so we can check that multi-layered logic works as expected
struct DoubleDecoratedStringifier : public DecoratedStringifierBase<DoubleDecoratedStringifier> {
    using base = DecoratedStringifierBase<DoubleDecoratedStringifier>;

    template <class T>
    static void append_float(std::string& buffer, const T& value) {
        buffer += '$';
        base::append_float(buffer, value);
        buffer += '$';
    }
};

TEST_CASE("Stringifier customization correctly decorates values") {
    CHECK(DecoratedStringifier{}(0.5) == "$0.5$");
    CHECK(DoubleDecoratedStringifier{}(0.5) == "$$0.5$$");

    CHECK(DecoratedStringifier{}(std::array{0.5, 1.5}) == "{ $0.5$, $1.5$ }");
}

// Create customization of stringifier that overrides formatting for specific type
struct OverridingStringifier : public log::StringifierBase<OverridingStringifier> {
    using base = log::StringifierBase<OverridingStringifier>;

    // Bring base class 'append()' here so we don't shadow it
    using base::append;

    // Declare overloads for types with specific formatting
    static void append(std::string& buffer, const std::vector<int>& value) {
        buffer += "integers: [ ";
        for (auto e : value) buffer += std::to_string(e) + " ";
        buffer += "]";
    }
};

TEST_CASE("Stringifier customization correctly overrides formatting of specific types") {
    CHECK(OverridingStringifier{}(std::array{1, 2, 3}) == "{ 1, 2, 3 }");
    CHECK(OverridingStringifier{}(std::vector{1, 2, 3}) == "integers: [ 1 2 3 ]");
    CHECK(OverridingStringifier{}(std::set{1, 2, 3}) == "{ 1, 2, 3 }");
}