#include "include/UTL/parallel.hpp"

#include <fstream>

int main() {
    using namespace utl;
    
    const std::string message = "<some hypothetically very large message>";
    
    // Log the message asynchronously
    parallel::detached_task([&]{ std::ofstream("log.txt") << message; });
    
    // ... do some work in the meantime ...
    
    // Destructor will automatically wait for ongoing tasks to finish before exiting 'main()'
}