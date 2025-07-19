#include "include/UTL/profiler.hpp"

void recursive(int depth = 0) {
    if (depth > 4) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return;
    }
    
    UTL_PROFILER("1st recursion branch") recursive(depth + 1);
    UTL_PROFILER("2nd recursion branch") recursive(depth + 2);
}

int main() {
    recursive();
}