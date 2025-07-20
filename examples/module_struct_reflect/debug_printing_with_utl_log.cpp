#include "include/UTL/struct_reflect.hpp"

#include "include/UTL/log.hpp"

// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

int main() {
    // Print struct
    using namespace utl;
    
    constexpr Quaternion q = { 0.5, 1.5, 2.5, 3.5 };
    
    log::println("q = ", struct_reflect::entry_view(q));
    
    // Note: there is no tight coupling between the modules, 
    //       'utl::log' just knows how to expand tuples,
    //       other loggers that do this will also work
}