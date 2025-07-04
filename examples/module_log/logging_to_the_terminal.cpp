#include "include/UTL/log.hpp"

#include <filesystem>
#include <map>
#include <set>
#include <vector>

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    // Create some complex objects that need logging
    auto vertex_ids = std::vector{4, 8, 17};
    auto success    = true;
    auto weights    = std::map{ std::pair{ "left_bc", 0.13}, std::pair{"right_bc", 0.34} };
    auto solver     = std::filesystem::path{"/usr/bin/solver"};
    auto state      = std::tuple{ "STRUCT_172", std::set{0, -2} };
    
    // Log stuff to console
    UTL_LOG_TRACE("Received boundary condition for edges ", vertex_ids);
    UTL_LOG_TRACE("Set up status: ", success, ", computing proper weights...");
    std::this_thread::sleep_for(75ms);
    
    UTL_LOG_INFO("Done! BC weights are: ", weights);
    UTL_LOG_TRACE("Starting solver [", solver, "] with state ", state, "...");
    std::this_thread::sleep_for(120ms);
    
    UTL_LOG_WARN("Low element quality, solution might be unstable");
    UTL_LOG_TRACE("Err -> ", log::PadLeft{1.2e+3, 8});
    UTL_LOG_TRACE("Err -> ", log::PadLeft{1.7e+5, 8});
    UTL_LOG_TRACE("Err -> ", log::PadLeft{4.8e+8, 8});
    UTL_LOG_ERR("The solver has burst into flames!");
    
    // no sinks were specified => 'std::cout' chosen by default
}