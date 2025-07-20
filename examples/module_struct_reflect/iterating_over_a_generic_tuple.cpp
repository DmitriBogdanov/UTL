#include "include/UTL/struct_reflect.hpp"

#include <string>
#include <iostream>

int main() {
    using namespace utl;
    
    std::tuple<std::string, int   > tuple_1{ "lorem", 2 };
    std::tuple<const char*, double> tuple_2{ "ipsum", 3 };
    
    // Print tuple
    struct_reflect::tuple_for_each(tuple_1, [&](auto&& x){ std::cout << x << '\n'; });
    
    // Print tuple sum
    struct_reflect::tuple_for_each(tuple_1, tuple_2, [&](auto&& x, auto&& y){ std::cout << x + y << '\n'; });
    
    // notice that tuples don't have to be homogenous,
    // what matters is that binary function can be called on all corresponding pairs
}