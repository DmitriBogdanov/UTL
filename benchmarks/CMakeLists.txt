# Macro for defining a benchmark with proper compile options
macro(add_utl_benchmark filename)
    add_executable(${filename} ${filename}.cpp)
    target_include_directories(${filename} PRIVATE "thirdparty")
    target_compile_features(${filename} PRIVATE cxx_std_17)
    target_compile_options(${filename} PRIVATE -O2 -Wall -Wextra -Wpedantic -Werror -fno-unroll-loops)
endmacro()

add_utl_benchmark(benchmark_json)
add_utl_benchmark(benchmark_log)
add_utl_benchmark(benchmark_mvl)
add_utl_benchmark(benchmark_parallel)
add_utl_benchmark(benchmark_profiler)
add_utl_benchmark(benchmark_random)

# Link OpenMP if doing benchmarks with it.
# Don't forget to add '-fopenmp' to 'target_compile_options' 
# find_package(OpenMP REQUIRED)
# target_link_libraries(benchmark_parallel PRIVATE OpenMP::OpenMP_CXX)

# Profiling flags for 'perf':
# -O2 -g -ggdb -fno-omit-frame-pointer