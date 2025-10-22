#include "include/UTL/stre.hpp"

#include <cassert>

int main() {
    using namespace utl;
    
    assert(stre::replace_all  ("__xx__xx__", "xx", "yy") == "__yy__yy__");
    assert(stre::replace_first("__xx__xx__", "xx", "yy") == "__yy__xx__");
    assert(stre::replace_last ("__xx__xx__", "xx", "yy") == "__xx__yy__");
    
    assert(stre::replace_prefix("__xx__xx__", "__xx", "--yy") == "--yy__xx__");
    assert(stre::replace_suffix("__xx__xx__", "xx__", "yy--") == "__xx__yy--");
}