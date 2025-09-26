#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>

int main() {
    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); });
    
    // Second argument is optional, this can be used like a regular assert
    ASSERT(2 + 4 == 17);
    
    return EXIT_FAILURE; // shouldn't be reached
}