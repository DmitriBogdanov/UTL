#include "include/UTL/describe_struct.hpp"

#include "include/UTL/log.hpp"

// Define struct & reflection
struct quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_DESCRIBE_STRUCT(quaternion, r, i, j, k);

int main() {
    // Print struct
    using namespace utl;
    
    constexpr quaternion q = { 0.5, 1.5, 2.5, 3.5 };
    
    log::println("q = ", describe_struct::entry_view(q));
    
    // Note: there is no tight coupling between the modules, 
    //       'utl::log' just knows how to expand tuples,
    //       other loggers that do this will also work
}