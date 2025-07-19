#include "include/UTL/profiler.hpp"

using namespace std::chrono_literals;

void computation_1() { std::this_thread::sleep_for(300ms); }
void computation_2() { std::this_thread::sleep_for(200ms); }
void computation_3() { std::this_thread::sleep_for(400ms); }
void computation_4() { std::this_thread::sleep_for(600ms); }
void computation_5() { std::this_thread::sleep_for(100ms); }

int main() {
    // Profile a scope
    UTL_PROFILER_SCOPE("Computation 1 - 5");
    computation_1();
    computation_2();
    
    // Profile an expression
    UTL_PROFILER("Computation 3") computation_3();
    
    // Profile a code segment
    UTL_PROFILER_BEGIN(comp_45, "Computation 4 - 5");
    computation_4();
    computation_5();
    UTL_PROFILER_END(comp_45);
}