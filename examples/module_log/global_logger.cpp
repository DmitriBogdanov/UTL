#include "include/UTL/log.hpp"

using namespace utl;

// Create global logger
auto& logger() {
    static auto instance = log::logger{
        log::sink{"log.txt"},
        log::sink{std::cout}
    };
    
    return instance;
}

int main() {
    // Use it
    logger().info("Message");
}