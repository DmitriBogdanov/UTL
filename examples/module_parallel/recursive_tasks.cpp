#include "include/UTL/parallel.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    // Deeply recursive illustrative task, not a practical way of computing fibonacci numbers
    std::function<int(int)> fibonacci = [&](int n) {
        if (n < 2) return n;
    
        auto future_prev_1 = parallel::awaitable_task([&] { return fibonacci(n - 1); });
        auto future_prev_2 = parallel::awaitable_task([&] { return fibonacci(n - 2); });
    
        return future_prev_1.get() + future_prev_2.get();
    };
    
    assert( fibonacci(12) == 144 );
}