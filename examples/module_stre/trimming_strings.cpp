#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::trim_left ("   lorem ipsum   ") ==    "lorem ipsum   ");
    assert(stre::trim_right("   lorem ipsum   ") == "   lorem ipsum"   );
    assert(stre::trim      ("   lorem ipsum   ") ==    "lorem ipsum"   );
    
    assert(stre::trim("__ASSERT_MACRO__", '_') == "ASSERT_MACRO");
}