#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
    assert(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
}