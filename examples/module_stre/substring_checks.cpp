#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    static_assert(stre::starts_with("lorem ipsum", "lorem"));
    static_assert(stre::ends_with  ("lorem ipsum", "ipsum"));
    static_assert(stre::contains   ("lorem ipsum", "em ip"));
}