#include "include/UTL/sleep.hpp"

#include <iostream>

int main() {
    using ms = std::chrono::duration<double, std::milli>;
    
    constexpr int repeats        = 6;
    constexpr ms  sleep_duration = ms(16.67);
    
    const auto measure_time = [](auto sleep_function) {
        for (int i = 0; i < repeats; ++i) {
            const auto start = std::chrono::steady_clock::now();
            sleep_function(sleep_duration);
            const auto end   = std::chrono::steady_clock::now();
            std::cout << ms(end - start).count() << " ms\n";
        }
    };
    
    std::cout << "Sleeping for 16.67 ms.\n";
    
    std::cout << "\n--- sleep::system()   ---\n";
    measure_time(utl::sleep::system<double, std::milli>);
    
    std::cout << "\n--- sleep::spinlock() ---\n";
    measure_time(utl::sleep::spinlock<double, std::milli>);
    
    std::cout << "\n--- sleep::hybrid()   ---\n";
    measure_time(utl::sleep::hybrid<double, std::milli>);
}