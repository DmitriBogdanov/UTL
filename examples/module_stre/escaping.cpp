#include "include/UTL/stre.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace utl;
    
    const std::string text = "this text\r will get messed up due to\r carriage returns.";
    
    std::cout
        << "Original string prints like this:\n" <<              text  << "\n\n"
        << "Escaped  string prints like this:\n" << stre::escape(text) << "\n\n";
}