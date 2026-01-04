#include "include/UTL/describe_struct.hpp"

#include <iostream>

using namespace utl;

// Define structure
struct vector { double x, y, z; }; UTL_DESCRIBE_STRUCT(vector, x, y, z);

int main() {
    
    constexpr vector vec = { 0.5, 1.5, 2.5 };
    
    // Serialize members
    describe_struct::for_indices<vector>([&] (auto i) {
        std::cout
            << "[" << i << "]: "
            << describe_struct::label<i>(vec) << " -> " << describe_struct::value<i>(vec)
            << "\n";
    });
    
}