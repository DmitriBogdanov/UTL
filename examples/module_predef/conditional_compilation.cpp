#include "include/UTL/predef.hpp"

#include <iostream>

int main() {
    #if defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
    std::cout << "Running Clang or GCC";
    #elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
    std::cout << "Running MSVC";
    #else
    std::cout << "Running some other compiler";
    #endif
}