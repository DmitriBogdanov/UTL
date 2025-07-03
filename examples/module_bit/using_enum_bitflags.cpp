#include "include/UTL/bit.hpp"

#include <iostream>

using namespace utl;

// Bitflag-suitable enum class
enum class IOMode { IN = 1 << 0, OUT = 1 << 1, APP = 1 << 2 };

// Function taking enum flags
void open_file(bit::Flags<IOMode> flags) {
    if (flags.contains(IOMode::IN) ) std::cout << "  > File opened for reading   \n";
    if (flags.contains(IOMode::OUT)) std::cout << "  > File opened for writing   \n";
    if (flags.contains(IOMode::APP)) std::cout << "  > File opened for appending \n";
}

int main() { 
    std::cout << "Opening file with OUT:       \n";
    open_file(IOMode::OUT);
    
    std::cout << "Opening file with OUT | APP: \n";
    open_file(bit::Flags{IOMode::OUT, IOMode::APP});
}