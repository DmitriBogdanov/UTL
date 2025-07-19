#include "include/UTL/stre.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace utl;
    
    // Repeating chars/strings
    assert(stre::repeat_char  (  'h', 7) == "hhhhhhh"        );
    assert(stre::repeat_string("xo-", 5) == "xo-xo-xo-xo-xo-");
    
    // Escaping control chars in a string   
    const std::string text = "this text\r will get messed up due to\r carriage returns.";
    std::cout
        << "Original string prints like this:\n" <<                            text  << "\n\n"
        << "Escaped  string prints like this:\n" << stre::escape_control_chars(text) << "\n\n";
    
    // Getting index of difference
    assert(stre::index_of_difference("xxxAxx", "xxxxxx") == 3);
}