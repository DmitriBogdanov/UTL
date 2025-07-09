#include "include/UTL/parallel.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    const std::vector<double> vals(200'000, 2);
    
    // Reduce container over a binary operation
    const double sum = parallel::reduce(vals, parallel::sum<double>());
    
    assert( sum == 200'000 * 2 );
    
    // Reduce range over a binary operation
    const double subrange_sum = parallel::reduce(parallel::Range{vals.begin() + 100, vals.end()}, parallel::sum<>());
    
    assert( subrange_sum == (200'000 - 100) * 2 );
}