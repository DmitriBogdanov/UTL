#include "include/UTL/random.hpp"

#include "include/UTL/log.hpp"

using namespace utl;

template <std::size_t size>
constexpr auto random_integers(std::uint64_t seed, int min, int max) {
    std::array<int, size>          res{};
    random::UniformIntDistribution dist{min, max};
    random::PRNG                   gen(seed);
    
    for (auto &e : res) e = dist(gen);
    return res;
}

int main() {
    constexpr auto random_array = random_integers<6>(0, -10, 10);
    
    log::println(random_array); // using another module for convenience
    
    // compile-time random like this can be used to automatically build
    // lookup tables and generate seemingly random patterns
}