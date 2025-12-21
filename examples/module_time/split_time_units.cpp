#include "include/UTL/time.hpp"

#include <iostream>

int main() {

    using namespace utl;
    
    constexpr auto duration = std::chrono::nanoseconds{ 1'700'400'300 };
    constexpr auto split    = time::unit_split(duration);
    
    static_assert( split.hours.count() ==   0 );
    static_assert( split.min  .count() ==   0 );
    static_assert( split.sec  .count() ==   1 );
    static_assert( split.ms   .count() == 700 );
    static_assert( split.us   .count() == 400 );
    static_assert( split.ns   .count() == 300 );
    
    std::cout << time::to_string(duration) << '\n';
}