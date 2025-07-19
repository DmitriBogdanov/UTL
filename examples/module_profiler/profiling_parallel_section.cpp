#include "include/UTL/profiler.hpp"

#include "include/UTL/parallel.hpp"

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    // Run loop on the main thread
    UTL_PROFILER("Single-threaded loop")
    for (int i = 0; i < 30; ++i) std::this_thread::sleep_for(10ms);
    
    // Run the same loop on 3 threads
    parallel::set_thread_count(3);
    
    UTL_PROFILER("Multi-threaded loop")
    parallel::blocking_loop(parallel::IndexRange{0, 30}, [](int low, int high){
        UTL_PROFILER("Worker thread loop")
        for (int i = low; i < high; ++i) std::this_thread::sleep_for(10ms);
    });
    
    parallel::set_thread_count(0);
}