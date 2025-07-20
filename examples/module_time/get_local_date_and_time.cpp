#include "include/UTL/time.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    std::cout
        << "Current date:     " << time::datetime_string("%y-%m-%d") << '\n'
        << "Current time:     " << time::datetime_string("%H:%M:%S") << '\n'
        << "Current datetime: " << time::datetime_string()           << '\n';
}