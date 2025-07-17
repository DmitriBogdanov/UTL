#include "include/UTL/parallel.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    // In debugging it is often useful to have some information about the thread whereabouts,
    // we can detect if current thread belongs to a pool, which pool and at which index
    auto future = parallel::awaitable_task([]{
        std::cout << "I am a thread [" << *parallel::this_thread::get_index() << "]"
                  << " in the pool  [" << *parallel::this_thread::get_pool()  << "]" 
                  << std::endl;
    });
    future.wait();
    
    auto std_future = std::async([]{
        std::cout << "Am I a pool thread? -> " << (parallel::this_thread::get_pool() ? "true" : "false")
                  << std::endl;
    });
    std_future.wait();
}