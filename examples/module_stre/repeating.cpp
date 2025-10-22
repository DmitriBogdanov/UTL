#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::repeat(  'h', 7) == "hhhhhhh"        );
    assert(stre::repeat("xo-", 5) == "xo-xo-xo-xo-xo-");
}