#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::markdown table({"Method", "Error", "Converged"});
    
    // 1 call to 'cell()' doesn't necessarily have to fill the entire row at once
    table.cell("Jacobi");
    table.cell(3.475e-4);
    table.cell(false);
    
    table.cell("Seidel");
    table.cell(6.732e-6, true);
    
    std::cout << table.format();
}