#define UTL_ASSERTION_ENABLE_SHORTCUT
#include "include/UTL/assertion.hpp"

#include <csignal>
#include <memory>

const auto before_main = [](){
    return std::signal(SIGABRT, [](int){
        std::cerr << "\nReceived SIGABRT\n";
        std::exit(EXIT_SUCCESS);
    });
}();

int main() {
    std::unique_ptr<int> component;
    
    ASSERT(component.get(), "Cannot invoke handling for an empty component.");
}