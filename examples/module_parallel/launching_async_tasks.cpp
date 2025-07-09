#include "include/UTL/parallel.hpp"

#include <fstream>

int main() {
    using namespace utl;
    
    const std::string message = "<some hypothetically very large message>";
    
    // Log the message asynchronously
    parallel::task([&]{ std::ofstream("log.txt") << message; });
    
    // ... do some other work in the meantime ...
    
    // Destructor will automatically wait for ongoing tasks to finish
    // before exiting 'main()', we can also trigger the wait manually
    parallel::wait_for_tasks();
}