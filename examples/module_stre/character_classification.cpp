#include "include/UTL/stre.hpp"

int main() {
    using namespace utl;
    
    static_assert(!stre::is_control( '5'));
    static_assert( stre::is_control('\f'));
    static_assert( stre::is_control('\n'));
    
    static_assert(!stre::is_graphical(' '));
    static_assert( stre::is_graphical('X'));
}