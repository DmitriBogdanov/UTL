// __________________________________ CONTENTS ___________________________________
//
//    Compile-time test that checks the library for ODR violations,
//    should any of the headers miss an 'inline' this will not build
//    due to including the same thing in multiple translation units.
// _______________________________________________________________________________

#include "second.hpp"

int main() {
    print_hello();
}