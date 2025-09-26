#define UTL_ASSERTION_ENABLE_SHORTCUT
#define UTL_ASSERTION_ENABLE_IN_RELEASE
#include "include/UTL/assertion.hpp"

#include <csignal>
#include <memory>

int main() {
    std::signal(SIGABRT, [](int){ std::quick_exit(EXIT_SUCCESS); });
    
    std::unique_ptr<int> component;
    
    ASSERT(component.get(), "Cannot invoke handling for an empty component.");
    
    return EXIT_FAILURE; // shouldn't be reached
}