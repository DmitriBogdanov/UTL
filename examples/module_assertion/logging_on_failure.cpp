#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>
#include <fstream>

int main() {
	#ifdef _WIN32
	 _set_abort_behavior(0, _WRITE_ABORT_MSG); // prevents Windows abort pop-up from appearing
	#endif

    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); }); // treat assert trigger as success
    
    utl::assertion::set_handler([](const utl::assertion::FailureInfo& info) {
        // Forward assertion message to some logging facility with colors disabled
        std::ofstream("failure.txt") << info.to_string();
        
        // Print & abort like usually
        std::cerr << info.to_string(true);
        std::abort();
    });
    
    ASSERT(3 + 4 < 6);
    
    return EXIT_FAILURE; // treat no assert trigger as failure
}