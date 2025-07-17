#include "include/UTL/parallel.hpp"

#include <cassert>

double some_heavy_computation(double x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x + 32;
}

int main() {
    using namespace utl;

    // Launch asynchronous computation
    auto future = parallel::awaitable_task(some_heavy_computation, 10);

    // ... do some work in the meantime ...

    // Await the result
    const double result = future.get();

    assert( result == 42 );
}