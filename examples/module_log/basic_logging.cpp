#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    // Log with a default global logger
    log::trace("Message 1");
    log::info ("Message 2");
    log::warn ("Message 3");
}