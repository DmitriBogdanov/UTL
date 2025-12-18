#include "include/UTL/bit.hpp"

#include <iostream>

using namespace utl;

// Bitflag-suitable enum class
enum class io_mode { in = 1 << 0, out = 1 << 1, app = 1 << 2 };

// Function taking enum flags
void open_file(bit::flags<io_mode> flags) {
    if (flags.contains(io_mode::in )) std::cout << "  > File opened for reading   \n";
    if (flags.contains(io_mode::out)) std::cout << "  > File opened for writing   \n";
    if (flags.contains(io_mode::app)) std::cout << "  > File opened for appending \n";
}

int main() { 
    std::cout << "Opening file with 'out':       \n";
    open_file(io_mode::out);
    
    std::cout << "Opening file with 'out' | 'app': \n";
    open_file(bit::flags{io_mode::out, io_mode::app});
}