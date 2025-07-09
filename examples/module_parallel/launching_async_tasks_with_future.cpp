#include "include/UTL/parallel.hpp"

#include <cassert>

double some_heavy_computation(double x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x + 32;
}

int main() {
    using namespace utl;

    // Launch the computation asynchronously and get its future
    auto future = parallel::task_with_future(some_heavy_computation, 10);

    // ... do some other work in the meantime ...

    // Get the value from std::future, this will wait until the task is finished
    const double result = future.get();

    assert( result == 42 );
}