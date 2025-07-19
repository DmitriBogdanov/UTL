#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::starts_with("lorem ipsum", "lorem"));
    assert(stre::ends_with  ("lorem ipsum", "ipsum"));
    assert(stre::contains   ("lorem ipsum", "em ip"));
}