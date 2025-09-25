#define UTL_ASSERTION_ENABLE_SHORTCUT
#include "include/UTL/assertion.hpp"

#include <csignal>
#include <fstream>

const auto before_main = [](){
    return std::signal(SIGABRT, [](int){
        std::cerr << "\nReceived SIGABRT\n";
        std::exit(EXIT_SUCCESS);
    });
}();

int main() {
    utl::assertion::set_handler([](const utl::assertion::FailureInfo& info) {
        // Forward assertion message to some logging facility with colors disabled
        std::ofstream("failure.txt") << info.to_string();
        
        // Print & abort like usually
        std::cerr << info.to_string(true);
        std::abort();
    });
    
    ASSERT(3 + 4 < 6);
}