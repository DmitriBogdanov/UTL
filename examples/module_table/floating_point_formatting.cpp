#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    const auto format_number = [](double x) { return table::numeric{x, std::chars_format::scientific, 1}; };
    
    table::markdown table({"Method", "Error"});
    
    table.cell("Jacobi", format_number(3.475e-4));
    table.cell("Seidel", format_number(6.732e-6));
    
    std::cout << table.format();
}