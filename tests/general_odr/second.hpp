#pragma once

#include "single_include/UTL.hpp"
// this will be included in both translation units, should
// there be any ODR violations the compiler will complain
//
// we include the automatically amalgamated header so we don't have 
// to update this test every time there is a new module, this also
// has a benefit of testing the amalgamated include in general

void print_hello();