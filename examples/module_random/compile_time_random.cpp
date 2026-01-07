#include "include/UTL/random.hpp"

#include "include/UTL/log.hpp"

using namespace utl;

template <std::size_t size>
constexpr auto random_integers(std::uint64_t seed, int min, int max) {
    std::array<int, size>            res {          };
    random::uniform_int_distribution dist{ min, max };
    random::default_generator        gen { seed     };
    
    for (auto &element : res) element = dist(gen);
    
    return res;
}

int main() {
    constexpr auto random_array = random_integers<6>(13, -8, 8);
    
    static_assert( random_array[0] == -8 );
    static_assert( random_array[1] == -7 );
    static_assert( random_array[2] ==  1 );
    static_assert( random_array[3] ==  8 );
    static_assert( random_array[4] == -3 );
    
    // compile-time random like this can be used to automatically build
    // lookup tables and generate seemingly random patterns, generated
    // sequence is platform-independent so we can test statically it
}