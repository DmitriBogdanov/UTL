#include "include/UTL/json.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    // Serialize JSON
    json::Node config;
    
    config["auxiliary_info"]       = true;
    config["date"]                 = "2024.04.02";
    config["options"]["grid_size"] = 120;
    config["options"]["phi_order"] = 5;
    config["scaling_functions"]    = { "identity", "log10" };
    config["time_steps"]           = 500;
    config["time_period"]          = 1.24709e+2;
    
    config.to_file("config.json");
    
    // Parse JSON
    config = json::from_file("config.json");
    
    std::cout << config.to_string();
}