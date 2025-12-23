#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::csv table(4);
    
    table.cell("Task", "Time", "Error", "Done");
    table.cell("Work 1", 1.35, 3.7e-5, true );
    table.cell("Work 2", 1.35, 2.5e-8, false);
    
    std::cout << table.format();
}