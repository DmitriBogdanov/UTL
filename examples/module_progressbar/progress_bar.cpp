#include "include/UTL/progressbar.hpp"

#include <thread>

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    const int  iterations = 50;
    const auto some_work  = [] { std::this_thread::sleep_for(10ms); };
    
    progressbar::Percentage bar;
    for (int i = 0; i < iterations; ++i) {
        some_work();
        bar.set_progress((i + 1.) / iterations);
    }
    bar.finish();
}