#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Log with a default global logger
    log::info("Message 1");
    log::warn("Message 2");
    log::err ("Message 3");
}