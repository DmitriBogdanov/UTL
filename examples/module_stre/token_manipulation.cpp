#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    // Replacing tokens
    assert(stre::replace_all("xxxAAxxxAAxxx",  "AA",  "BBB") == "xxxBBBxxxBBBxxx" );
    
    // Splitting by delimiter
    auto tokens = stre::tokenize("aaa,bbb,ccc,", ",");
    assert(tokens.size() == 3);
    assert(tokens[0] == "aaa");
    assert(tokens[1] == "bbb");
    assert(tokens[2] == "ccc");
    
    // Splitting by complex delimiter while keeping the empty tokens
    tokens = stre::split("(---)lorem(---)ipsum", "(---)");
    assert(tokens.size() == 3);
    assert(tokens[0] == ""     );
    assert(tokens[1] == "lorem");
    assert(tokens[2] == "ipsum");
}