#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::pad_left ("value", 9) == "    value" );
    assert(stre::pad_right("value", 9) == "value    " );
    assert(stre::pad      ("value", 9) == "  value  " );
    
    assert(stre::pad(" label ", 15, '-') == "---- label ----");
}