#include "include/UTL/progressbar.hpp"

#include <thread>

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    const int  work_size = 50;
    const auto work_unit = [] { std::this_thread::sleep_for(10ms); };
    
    progressbar::percentage bar;
    
    for (int i = 0; i < work_size; ++i) {
        work_unit();
        bar.set_progress((i + 1.) / work_size);
    }
    
    bar.finish();
}