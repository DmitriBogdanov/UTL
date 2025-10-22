#include "include/UTL/stre.hpp"

int main() {
    using namespace utl;
    
    static_assert(stre::first_difference("xxxxYx", "xxxxXx") == 4);
    static_assert(stre::first_difference("xxx"   , "xxxxxx") == 3);
    
    static_assert(stre::count_difference("xxxxYx", "xxxxXx") == 1);
    static_assert(stre::count_difference("yyy"   , "xxxxxx") == 6);
}