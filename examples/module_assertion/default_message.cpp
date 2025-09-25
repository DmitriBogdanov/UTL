#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_THROW_ON_FAILURE
#include "include/UTL/assertion.hpp"

#include <csignal>

const auto before_main = [](){
    return std::signal(SIGABRT, [](int){
        std::cerr << "\nReceived SIGABRT\n";
        std::exit(EXIT_SUCCESS);
    });
}();

int main() {
    // Second argument is optional, this can be used like a regular assert
    ASSERT(2 + 4 == 17);
}