#include "include/UTL/reflect_struct.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    // Define structure
    struct vector { double x, y, z; };
    
    constexpr vector vec = { 0.5, 1.5, 2.5 };
    
    // Serialize members
    reflect_struct::for_indices<vector>([&] (auto i) {
        std::cout << "[" << i << "]: " << reflect_struct::label<i>(vec) << " -> " << reflect_struct::value<i>(vec) << "\n";
    });
}