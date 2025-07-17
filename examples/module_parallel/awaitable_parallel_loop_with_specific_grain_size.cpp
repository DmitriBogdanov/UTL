#include "include/UTL/parallel.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    std::vector<int> a(200'000, 27);
    std::vector<int> b(200'000, 13);
    std::vector<int> c(200'000,  0);
    
    // Launch the task to compute { c = a + b } in parallel, we know this
    // workload is very even so we can use coarser grains that by default
    const std::size_t grain_size = 200'000 / parallel::get_thread_count();
    
    auto future = parallel::awaitable_loop(parallel::IndexRange<std::size_t>{0, 200'000, grain_size},
        [&](std::size_t i){ c[i] = a[i] + b[i]; }
    );
    
    // ... do some work in the meantime ...
    
    // Await the result
    future.wait();
    
    for (std::size_t i = 0; i < 200'000; ++i) assert( c[i] == a[i] + b[i] );
}