#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>

int main() {
    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); });
    
    const int rows = 10;
    const int cols = 12;
    
    ASSERT(rows == cols, "Linear system requires a square matrix.");
    
    return EXIT_FAILURE; // shouldn't be reached
}