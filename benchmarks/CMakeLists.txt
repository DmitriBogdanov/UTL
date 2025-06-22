# Macro for defining a benchmark with proper args
macro(utl_add_benchmark filename)
    add_executable(${filename} ${filename}.cpp)
    
    target_compile_features(${filename} PRIVATE ${UTL_COMPILE_FEATURES})
    target_compile_options (${filename} PRIVATE ${UTL_BENCHMARK_FLAGS} )
    
    # Note: Benchmarking against OpenMP requires adding '-fopenmp' to the flags and
    #       then linking 'target_link_libraries(${filename} PRIVATE OpenMP::OpenMP_CXX)'
    
endmacro()

# Benchmarks
utl_add_benchmark(benchmark_json)
utl_add_benchmark(benchmark_log)
utl_add_benchmark(benchmark_mvl)
utl_add_benchmark(benchmark_parallel)
utl_add_benchmark(benchmark_profiler)
utl_add_benchmark(benchmark_random)
