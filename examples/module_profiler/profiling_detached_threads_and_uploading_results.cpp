#include "include/UTL/profiler.hpp"

#include "include/UTL/parallel.hpp"

int main() {
    using namespace utl;
    using namespace std::chrono_literals;
    
    parallel::set_thread_count(2);
    
    // Detached task
    UTL_PROFILER("Uploading task 1")
    parallel::detached_task([]{
        UTL_PROFILER("Detached task 1: part 1") std::this_thread::sleep_for(700ms);
    });
    
    // Detached task with explicit result upload
    UTL_PROFILER("Uploading task 2")
    parallel::detached_task([]{
        UTL_PROFILER("Detached task 2: part 1") std::this_thread::sleep_for(50ms);
        UTL_PROFILER("Detached task 2: part 2") std::this_thread::sleep_for(50ms);
    
        // Manually upload results to the main thread,
        // otherwise results get collected once the thread joins
        profiler::profiler.upload_this_thread();
    
        UTL_PROFILER("Detached task 2: part 3") std::this_thread::sleep_for(500ms);
    });
    
    // Wait a little so the 2nd task has time to reach manual upload
    UTL_PROFILER("Waiting for task 2 to be partially done")
    std::this_thread::sleep_for(200ms);
    
    // Format results explicitly
    profiler::profiler.print_at_exit(false);
    
    std::cout << profiler::profiler.format_results();
}