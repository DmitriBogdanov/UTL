#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>
#include <fstream>

int main() {
    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); });
    
    utl::assertion::set_handler([](const utl::assertion::FailureInfo& info) {
        // Forward assertion message to some logging facility with colors disabled
        std::ofstream("failure.txt") << info.to_string();
        
        // Print & abort like usually
        std::cerr << info.to_string(true);
        std::abort();
    });
    
    ASSERT(3 + 4 < 6);
    
    return EXIT_FAILURE; // shouldn't be reached
}