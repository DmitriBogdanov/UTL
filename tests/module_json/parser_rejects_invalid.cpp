#include "tests/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

#include <unordered_set> // unordered_set<>

// ____________________ IMPLEMENTATION ____________________

// Parser RFC-8259 conformance testing based on https://github.com/nst/JSONTestSuite/

TEST_CASE("Parser rejects invalid") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_reject/";

    std::cout << "Running test case from path: " << fs::current_path() << std::endl;

    // JSON specification is rather strict when it comes to numeric format, not allowing a lot of the valid IEEE 754
    // floats that work fine with C++ formatting facilities. Since there is no real benefit in prohibiting the parsing
    // of such values, we explicitly mark them as "acceptable to parse".
    const std::unordered_set<std::string> acceptable_cases = {"n_number_0.e1.json",
                                                              "n_number_2.e3.json",
                                                              "n_number_2.e+3.json",
                                                              "n_number_2.e-3.json",
                                                              "n_number_-01.json",
                                                              "n_number_-2..json",
                                                              "n_number_-NaN.json",
                                                              "n_number_minus_infinity.json",
                                                              "n_number_neg_int_starting_with_zero.json",
                                                              "n_number_neg_real_without_int_part.json",
                                                              "n_number_real_without_fractional_part.json",
                                                              "n_number_with_leading_zero.json"};

    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        std::cout << "Parsing file named: " << test_suite_entry.path() << std::endl;

        if (!acceptable_cases.count(test_suite_entry.path().filename().string()))
            CHECK_THROWS(json::from_file(test_suite_entry.path().string()));
    }
}