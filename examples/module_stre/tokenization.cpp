#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    auto tokens = stre::tokenize("aaa,bbb,,ccc,", ","); // empty tokens are discarded
    assert(tokens.size() == 3);
    assert(tokens[0] == "aaa");
    assert(tokens[1] == "bbb");
    assert(tokens[2] == "ccc");
    
    tokens = stre::split("(---)lorem(---)ipsum", "(---)"); // empty tokens are preserved
    assert(tokens.size() == 3  );
    assert(tokens[0] == ""     );
    assert(tokens[1] == "lorem");
    assert(tokens[2] == "ipsum");
}