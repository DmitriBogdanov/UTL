#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Create local logger
    auto logger = log::Logger{
        log::Sink{"log.txt"},
        log::Sink{std::cout}
    };
    
    // Use it
    logger.info("Message");
}