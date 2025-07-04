#include "include/UTL/json.hpp"

#include <cassert>
#include <iostream>

struct Config {
    bool        auxiliary_info = true;
    std::string date           = "2024.04.02";

    struct Options {
        int grid_size = 120;
        int phi_order = 5;
    } options;

    std::vector<std::string> scaling_functions = {"identity", "log10"};
    std::size_t              time_steps        = 500;
    double                   time_period       = 1.24709e+2;
};

UTL_JSON_REFLECT(Config, auxiliary_info, date, options, scaling_functions, time_steps, time_period);
UTL_JSON_REFLECT(Config::Options, grid_size, phi_order);

int main() {
    using namespace utl;

    // Parse JSON from struct
    auto       config = Config{};
    json::Node json   = json::from_struct(config);

    // Test the result
    std::cout << "--- Struct to JSON ---\n" << json.to_string();

    // Serialize JSON to struct
    auto serialized_config = json.to_struct<Config>();

    // Test the result
    assert( config.auxiliary_info    == serialized_config.auxiliary_info    );
    assert( config.date              == serialized_config.date              );
    assert( config.options.grid_size == serialized_config.options.grid_size );
    // ...and so on
}