#include "include/UTL/random.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    // Generic functions
    std::cout
        << "integer U[-5, 5] -> " << random::uniform(-5, 5)    << "\n"
        << "boolean U[0, 1]  -> " << random::uniform<bool>()   << "\n"
        << "float   U[1, 2)  -> " << random::uniform(1.f, 2.f) << "\n"
        << "float   U[0, 1)  -> " << random::uniform<float>()  << "\n";
    
    // Standard shortcuts
    std::cout
        << "U[0, 1] -> " << random::uniform_bool()  << "\n"
        << "N(0, 1) -> " << random::normal_double() << "\n";
    
    // Other distributions
    std::cout
        << "Exp(4) -> " << random::variate(std::exponential_distribution{4.f})  << "\n";
}