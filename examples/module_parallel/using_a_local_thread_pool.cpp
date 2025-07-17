#include "include/UTL/parallel.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    parallel::ThreadPool pool; // uses as many threads as it detects
    
    pool.detached_task([]{ std::cout << "Hello from the task\n"; });
    
    pool.set_thread_count(0); // will wait for the tasks, join all threads and release the memory
}