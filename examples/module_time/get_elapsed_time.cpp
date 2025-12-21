#include "include/UTL/time.hpp"

#include <thread>
#include <iostream>

int main() {
    using namespace utl;
    
    const auto some_work = []{ std::this_thread::sleep_for(time::sec(1.7)); };
    
    // Elapsed time as string
    time::stopwatch watch;
    some_work();
    std::cout << watch.elapsed_string() << '\n';
    
    // Elapsed time as double
    watch.start();
    some_work();
    std::cout << watch.elapsed_ms().count() << '\n';
}