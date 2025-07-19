#include "include/UTL/random.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    random::generators::SplitMix64 gen{random::entropy()};
    std::chi_squared_distribution distr{2.}; // Chi-squared distribution with N = 2
    
    std::cout << "Random value from distribution -> " << distr(gen) << "\n";
}