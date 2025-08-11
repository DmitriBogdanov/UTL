#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    utl::table::Markdown tb({"Method", "Error", "Converged"});
    
    // 1 call to 'cell()' doesn't necessarily have to fill the entire row at once
    tb.cell("Jacobi");
    tb.cell(3.475e-4);
    tb.cell(false);
    
    tb.cell("Seidel");
    tb.cell(6.732e-6, true);
    
    std::cout << tb.format();
}