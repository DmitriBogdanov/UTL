#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::Mathematica tb(4);
    
    tb.hline();
    tb.cell("Task", "Time", "Error", "Done");
    tb.hline();
    tb.cell("Work 1", 1.35, 3.7e-5, true );
    tb.cell("Work 2", 1.35, 2.5e-8, false);
    tb.hline();
    
    std::cout << tb.format();
}