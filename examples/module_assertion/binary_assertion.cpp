#define UTL_ASSERTION_ENABLE_SHORTCUT
#include "include/UTL/assertion.hpp"

#include <csignal>

const auto before_main = [](){
    return std::signal(SIGABRT, [](int){
        std::cerr << "\nReceived SIGABRT\n";
        std::exit(EXIT_SUCCESS);
    });
}();

int main() {
    const int rows = 10;
    const int cols = 12;
    
    ASSERT(rows == cols, "Linear system requires a square matrix.");
}