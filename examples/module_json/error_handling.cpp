#include "include/UTL/json.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    const auto invalid_json = R"(
        {
            "key_1": "value_1",
            "key_2":  value_2",
            "key_3": "value_3"
        }
    )";
    
    try {
        [[maybe_unused]] auto res = json::from_string(invalid_json);
    }
    catch (std::runtime_error &e) {
        std::cerr << "ERROR: Caught exception:\n\n" << e.what();
    }
}