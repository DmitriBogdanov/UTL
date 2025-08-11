#include "include/UTL/table.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    const auto format_number = [](double x) { return table::Number{x, std::chars_format::scientific, 1}; };
    
    table::Markdown tb({"Method", "Error"});
    
    tb.cell("Jacobi", format_number(3.475e-4));
    tb.cell("Seidel", format_number(6.732e-6));
    
    std::cout << tb.format();
}