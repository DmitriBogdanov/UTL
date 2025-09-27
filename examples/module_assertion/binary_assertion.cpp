#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>

int main() {
	#ifdef _WIN32
	 _set_abort_behavior(0, _WRITE_ABORT_MSG); // prevents Windows abort pop-up from appearing
	#endif

    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); }); // treat assert trigger as success
    
    const int rows = 10;
    const int cols = 12;

    ASSERT(rows == cols, "Linear system requires a square matrix.");
    
    return EXIT_FAILURE; // treat no assert trigger as failure
}