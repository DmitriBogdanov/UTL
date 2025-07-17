#include "include/UTL/parallel.hpp"

#include <cmath>

double f(double x) { return std::exp(std::sin(x)); }

int main() {
    using namespace utl;
    
    std::vector<double> vals(200'000, 0.5);
    
    // (optional) Select the number of threads 
    parallel::set_thread_count(8);
    
    // Apply f() to all elements of the vector
    parallel::blocking_loop(vals, [&](auto it) { *it = f(*it); });
    
    // Apply f() to indices [0, 100)
    parallel::blocking_loop(parallel::IndexRange{0, 100}, [&](int i) { vals[i] = f(vals[i]); });
    
    // Specify computation in blocks instead of element-wise
    parallel::blocking_loop(parallel::IndexRange{0, 100}, [&](int low, int high) {
        for (int i = low; i < high; ++i) vals[i] = f(vals[i]);
    });
}