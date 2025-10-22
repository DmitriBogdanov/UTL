#include "include/UTL/stre.hpp"

int main() {
    using namespace utl;
    
    static_assert(stre::trim_left ("   lorem ipsum   ") ==    "lorem ipsum   ");
    static_assert(stre::trim_right("   lorem ipsum   ") == "   lorem ipsum"   );
    static_assert(stre::trim      ("   lorem ipsum   ") ==    "lorem ipsum"   );
    
    static_assert(stre::trim("__ASSERT__", '_') == "ASSERT");
}