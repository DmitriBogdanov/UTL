#include "include/UTL/parallel.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    const std::vector<double> vals(200'000, 2);
    
    // Reduce container over a binary operation
    const double sum = parallel::blocking_reduce(vals, parallel::sum<>());
    
    assert( sum == 200'000 * 2 );
    
    // Reduce iterator range over a binary operation
    const double subrange_sum = parallel::blocking_reduce(parallel::Range{vals.begin() + 100, vals.end()}, parallel::sum<>{});
    
    assert( subrange_sum == (200'000 - 100) * 2 );
}