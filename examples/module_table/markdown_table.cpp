#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::Markdown tb({"Task", "Time", "Error", "Done"});
    
    tb.cell("Work 1", 1.35, 3.7e-5, true );
    tb.cell("Work 2", 1.35, 2.5e-8, false);
    
    std::cout << tb.format();
}