#include "include/UTL/log.hpp"

using namespace utl;

// Create global logger
auto& logger() {
    static auto logger = log::Logger{
        log::Sink{"log.txt"},
        log::Sink{std::cout}
    };
    
    return logger;
}

int main() {
    // Use it
    logger().info("Message");
}