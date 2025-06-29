#include "tests/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// Parser RFC-8259 conformance testing based on https://github.com/nst/JSONTestSuite/

TEST_CASE("Parser accepts valid") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_accept/";

    std::cout << "Running test case from path: " << fs::current_path() << '\n' << std::flush;

    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        std::cout << "Parsing file named: " << test_suite_entry.path() << '\n' << std::flush;

        CHECK_NOTHROW(json::from_file(test_suite_entry.path()));
    }
}