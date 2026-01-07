#include "include/UTL/random.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    // Generic functions
    std::cout << "integer U[3, 7] -> " << random::uniform(  3,   7) << "\n";
    std::cout << "float   U[1, 2) -> " << random::uniform(1.f, 2.f) << "\n";
    
    std::cout << "boolean U[0, 1] -> " << random::uniform< bool>() << "\n";
    std::cout << "float   U[0, 1) -> " << random::uniform<float>() << "\n";
    
    // Standard shortcuts
    std::cout << "float   U[0, 1] -> " << random::uniform_float() << "\n";
    std::cout << "double  N(0, 1) -> " << random::normal_double() << "\n";
    
    // Arbitrary distributions
    std::cout << "float   Exp(4)  -> " << random::variate(std::exponential_distribution{4.f}) << "\n";
}