#include "include/UTL/integral.hpp"

#include <iostream>

int main() {
    try {
        using namespace utl;

        // Narrow cast
        [[maybe_unused]] char c1 =           static_cast<char>(  34); // this is fine, returns 34
        [[maybe_unused]] char c2 = integral::narrow_cast<char>(  34); // this is fine, returns 34
        [[maybe_unused]] char c3 =           static_cast<char>(1753); // silently overflows, returns -39
        [[maybe_unused]] char c4 = integral::narrow_cast<char>(1753); // throws 'std::domain_error'
        
    } catch (std::domain_error &e) { 
        std::cerr << "ERROR: Caught exception:\n\n" << e.what();
    }
}