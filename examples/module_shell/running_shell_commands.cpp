#include "include/UTL/shell.hpp"

#include <cassert>

int main() {
    #ifdef __linux__
    
    const auto res = utl::shell::run_command("echo TEXT");
    // usually used to invoke scripts and other executables
    
    assert(res.status ==      0);
    assert(res.out    == "TEXT");
    assert(res.err    ==     "");
    
    #endif
}