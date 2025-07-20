#include "include/UTL/time.hpp"

#include <thread>
#include <iostream>

int main() {
    using namespace utl;
    
    const auto some_work = [] { std::this_thread::sleep_for(time::sec(0.05)); };
    
    // Accumulate time on 'some_work()' in a loop
    time::Stopwatch watch;
    time::ms        total{};
    
    for (std::size_t i = 0; i < 20; ++i) {
        watch.start();
        some_work();
        total += watch.elapsed();
    }
    
    std::cout << time::to_string(total, 2) << '\n';
}