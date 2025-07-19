#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    // Replacing tokens
    assert(stre::replace_all_occurrences("xxxAAxxxAAxxx",  "AA",  "BBB") == "xxxBBBxxxBBBxxx" );
    
    // Splitting by delimiter
    auto tokens = stre::split_by_delimiter("aaa,bbb,ccc,", ",");
    assert(tokens.size() == 3);
    assert(tokens[0] == "aaa");
    assert(tokens[1] == "bbb");
    assert(tokens[2] == "ccc");
    
    // Splitting by complex delimiter while keeping the empty tokens
    tokens = stre::split_by_delimiter("(---)lorem(---)ipsum", "(---)", true);
    assert(tokens.size() == 3);
    assert(tokens[0] == ""     );
    assert(tokens[1] == "lorem");
    assert(tokens[2] == "ipsum");
}