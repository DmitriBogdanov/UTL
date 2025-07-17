#include "include/UTL/parallel.hpp"

#include <fstream>

int main() {
    using namespace utl;
    
    const std::string message = "<some hypothetically very large message>";
    
    // Log the message asynchronously
    parallel::detached_task([&]{ std::ofstream("log.txt") << message; });
    
    // ... do some work in the meantime ...
    
    parallel::wait(); // wait for tasks to complete
}