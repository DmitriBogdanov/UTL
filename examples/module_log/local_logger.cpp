#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Create local logger
    auto logger = log::logger{
        log::sink{"log.txt"},
        log::sink{std::cout}
    };
    
    // Use it
    logger.info("Message");
}