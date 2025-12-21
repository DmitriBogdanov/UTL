#include "include/UTL/time.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    time::timer   timer;
    std::uint64_t count = 0;
    
    timer.start(time::sec{1});
    
    while (!timer.finished()) ++count;
    
    std::cout << "Counted to " << count << " while looping for " << timer.elapsed_string() << '\n';
}