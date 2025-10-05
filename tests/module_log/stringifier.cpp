#include "tests/common.hpp"

#include "include/UTL/log.hpp"

// _______________________ INCLUDES _______________________

#include <complex> // complex<>
#include <cstdint> // int8_t, int16_t, int32_t, int64_t, ...
#include <deque>   // deque<>
#include <map>     // map<>
#include <queue>   // queue<>, priority_queue<>
#include <set>     // set<>
#include <stack>   // stack<>
#include <tuple>   // tuple
#include <utility> // pair<>

// ____________________ IMPLEMENTATION ____________________

// Tests for stringification of various standard types and containers

TEST_CASE("Stringifier / Basic API") {
    const auto value = std::make_tuple(true, 'a', 1, 0.5, std::array{1, 2});

    const std::string expected_result = "< true, a, 1, 0.5, [ 1, 2 ] >";

    CHECK(log::stringify(value) == expected_result);
}

TEST_CASE("Stringifier / Bools") {
    CHECK(log::stringify(false) == "false");
    CHECK(log::stringify(true) == "true");
}

TEST_CASE("Stringifier / Strings") {

    struct StringViewConvertible {
        operator std::string_view() const { return "<string_view>"; }
    };

    struct StringConvertible {
        operator std::string() const { return "<string>"; }
    };

    // Char
    CHECK(log::stringify('g') == "g");

    // String view-convertible
    CHECK(log::stringify("lorem ipsum") == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"s) == "lorem ipsum");
    CHECK(log::stringify("lorem ipsum"sv) == "lorem ipsum");
    CHECK(log::stringify(StringViewConvertible{}) == "<string_view>");

    // String-convertible
    CHECK(log::stringify(fs::path("lorem/ipsum")) == "lorem/ipsum");
    CHECK(log::stringify(StringConvertible{}) == "<string>");
}

TEST_CASE("Stringifier / Integers") {
    CHECK(log::stringify(0) == "0");
    CHECK(log::stringify(-450) == "-450");

    CHECK(log::stringify(-17) == "-17");
    CHECK(log::stringify(17u) == "17");
    CHECK(log::stringify(-17l) == "-17");
    CHECK(log::stringify(17ul) == "17");
    CHECK(log::stringify(-17ll) == "-17");
    CHECK(log::stringify(17ull) == "17");

    CHECK(log::stringify(nl<std::int8_t>::min()) == std::to_string(nl<std::int8_t>::min()));
    CHECK(log::stringify(nl<std::int16_t>::min()) == std::to_string(nl<std::int16_t>::min()));
    CHECK(log::stringify(nl<std::int32_t>::min()) == std::to_string(nl<std::int32_t>::min()));
    CHECK(log::stringify(nl<std::int64_t>::min()) == std::to_string(nl<std::int64_t>::min()));

    CHECK(log::stringify(nl<std::int8_t>::max()) == std::to_string(nl<std::int8_t>::max()));
    CHECK(log::stringify(nl<std::int16_t>::max()) == std::to_string(nl<std::int16_t>::max()));
    CHECK(log::stringify(nl<std::int32_t>::max()) == std::to_string(nl<std::int32_t>::max()));
    CHECK(log::stringify(nl<std::int64_t>::max()) == std::to_string(nl<std::int64_t>::max()));

    CHECK(log::stringify(nl<std::uint8_t>::min()) == std::to_string(nl<std::uint8_t>::min()));
    CHECK(log::stringify(nl<std::uint16_t>::min()) == std::to_string(nl<std::uint16_t>::min()));
    CHECK(log::stringify(nl<std::uint32_t>::min()) == std::to_string(nl<std::uint32_t>::min()));
    CHECK(log::stringify(nl<std::uint64_t>::min()) == std::to_string(nl<std::uint64_t>::min()));

    CHECK(log::stringify(nl<std::uint8_t>::max()) == std::to_string(nl<std::uint8_t>::max()));
    CHECK(log::stringify(nl<std::uint16_t>::max()) == std::to_string(nl<std::uint16_t>::max()));
    CHECK(log::stringify(nl<std::uint32_t>::max()) == std::to_string(nl<std::uint32_t>::max()));
    CHECK(log::stringify(nl<std::uint64_t>::max()) == std::to_string(nl<std::uint64_t>::max()));
}

TEST_CASE("Stringifier / Floats") {
    CHECK(log::stringify(0.5) == "0.5");
    CHECK(log::stringify(-1.5) == "-1.5");
    CHECK(log::stringify(0.) == "0");

    CHECK_NOTHROW(log::stringify(nl<float>::max()));
    CHECK_NOTHROW(log::stringify(nl<float>::min()));
    CHECK_NOTHROW(log::stringify(nl<double>::max()));
    CHECK_NOTHROW(log::stringify(nl<double>::min()));
    CHECK_NOTHROW(log::stringify(nl<long double>::max()));
    CHECK_NOTHROW(log::stringify(nl<long double>::min()));
}

TEST_CASE("Stringifier / std::complex") {
    CHECK(log::stringify(std::complex<double>{1, 2}) == "1 + 2i");
    CHECK(log::stringify(std::complex<double>{4, 0}) == "4 + 0i");
    CHECK(log::stringify(std::complex<double>{1, -3}) == "1 - 3i");
    CHECK(log::stringify(std::complex<double>{-1, -3}) == "-1 - 3i");
}

TEST_CASE("Stringifier / Arrays") {
    CHECK(log::stringify(std::array{1, 2, 3}) == "[ 1, 2, 3 ]");
    CHECK(log::stringify(std::vector{1, 2, 3}) == "[ 1, 2, 3 ]");
    CHECK(log::stringify(std::set{1, 2, 3}) == "[ 1, 2, 3 ]");
    CHECK(log::stringify(std::deque{1, 2, 3}) == "[ 1, 2, 3 ]");
}

TEST_CASE("Stringifier / Tuples") {
    CHECK(log::stringify(std::pair{1, 2}) == "< 1, 2 >");
    CHECK(log::stringify(std::tuple{"lorem", 2, "ipsum"}) == "< lorem, 2, ipsum >");
}

TEST_CASE("Stringifier / Adaptors") {

    class CustomContainerAdaptor {
    public:
        using container_type = std::tuple<std::string, std::string>;

        CustomContainerAdaptor(container_type&& container) : c(container) {}

    protected:
        container_type c;
    };

    const auto dq = std::deque{1, 2, 3};
    const auto tp = std::tuple{"lorem", "ipsum"};

    CHECK(log::stringify(std::queue{dq}) == "[ 1, 2, 3 ]");
    CHECK(log::stringify(std::priority_queue{dq.begin(), dq.end()}) == "[ 3, 2, 1 ]");
    CHECK(log::stringify(std::stack{dq}) == "[ 1, 2, 3 ]");
    CHECK(log::stringify(CustomContainerAdaptor{tp}) == "< lorem, ipsum >");
}

TEST_CASE("Stringifier / Duration") {
    CHECK(log::stringify(std::chrono::microseconds(1'700'067)) == "1 sec 700 ms 67 us");
    CHECK(log::stringify(std::chrono::nanoseconds(0)) == "0 ns");
}

struct Printable {};

std::ostream& operator<<(std::ostream& os, Printable) { return os << "printable_value"; }

TEST_CASE("Stringifier / Printables") {
    CHECK(log::stringify(Printable{}) == "printable_value");
    // other printable types would behave similarly
}

TEST_CASE("Stringifier / Compound types") {
    // clang-format off
    CHECK(log::stringify(std::map{std::pair{"k1", 1},std::pair{"k2", 2}}) == "[ < k1, 1 >, < k2, 2 > ]");
    CHECK(log::stringify(std::vector{std::vector{1, 2},std::vector{3}}) == "[ [ 1, 2 ], [ 3 ] ]");
    CHECK(log::stringify(std::vector<std::vector<std::vector<const char*>>>{{{"lorem"}}}) == "[ [ [ lorem ] ] ]");
    // clang-format on
}