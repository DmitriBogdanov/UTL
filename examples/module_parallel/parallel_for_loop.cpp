#include "include/UTL/parallel.hpp"

#include <cmath>

double f(double x) { return std::exp(std::sin(x)); }

int main() {
    using namespace utl;

    std::vector<double> vals(400'000, 0.5);

    // (optional) Select the number of threads 
    parallel::set_thread_count(8);

    // Apply f() to all elements of the vector
    parallel::for_loop(vals, [&](auto low, auto high) {
        for (auto it = low; it != high; ++it) *it = f(*it);
    });

    // Apply f() to the fist half of the vector
    parallel::for_loop(parallel::IndexRange<std::size_t>{0, vals.size() / 2}, [&](auto low, auto high) {
        for (auto i = low; i != high; ++i) vals[i] = f(vals[i]);
    });
}