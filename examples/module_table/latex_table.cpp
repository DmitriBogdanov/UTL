#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::latex table(4);
    
    table.hline();
    table.cell("Task", "Time", "Error", "Done");
    table.hline();
    table.cell("Work 1", 1.35, 3.7e-5, true );
    table.cell("Work 2", 1.35, 2.5e-8, false);
    table.hline();
    
    std::cout << table.format();
}