#include "include/UTL/random.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    random::generators::splitmix_64 gen { random::entropy() }; // any random:: generator
    std::chi_squared_distribution   dist{ 2.0               }; // any standard distribution
    
    std::cout << "Chi-squared(2) -> " << dist(gen) << "\n";
}