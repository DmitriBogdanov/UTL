#include "include/UTL/profiler.hpp"

#include <fstream>

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    // Profile something
    UTL_PROFILER("Loop")
    for (int i = 0; i < 10; ++i) {
        UTL_PROFILER("1st half of the loop") std::this_thread::sleep_for(10ms);
        UTL_PROFILER("2nd half of the loop") std::this_thread::sleep_for(10ms);
    }
    
    // Disable automatic printing
    profiler::profiler.print_at_exit(false);
    
    // Disable colors, remove indent, format to string
    profiler::Style style;
    style.color  = false;
    style.indent = 0;
    
    const std::string results = profiler::profiler.format_results(style);
    
    // Export to file & console
    std::ofstream("profiling_results.txt") << results;
    std::cout                              << results;
}