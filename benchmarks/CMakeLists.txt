# Function for defining a benchmark with proper args
function(utl_add_benchmark file_name)
    string(REPLACE "/" "-" target_name "${file_name}") # CMake doesn't like target names with slashes
    string(PREPEND target_name "benchmark-") # so we don't intersect names with tests & examples
    
    add_executable(${target_name} ${file_name}.cpp)
    
    target_compile_features   (${target_name} PRIVATE ${UTL_COMPILE_FEATURES}      )
    target_compile_options    (${target_name} PRIVATE ${UTL_BENCHMARK_FLAGS}       )
    target_link_options       (${target_name} PRIVATE ${UTL_BENCHMARK_LINKER_FLAGS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}          )
    
endfunction()

# Benchmarks
utl_add_benchmark("module_json/parse_serialize")
utl_add_benchmark("module_log/logging_overhead")
utl_add_benchmark("module_log/stringification")
utl_add_benchmark("module_mvl/experimental")
# utl_add_benchmark("module_parallel/parallel_repeated_matmul") // TODO:
# utl_add_benchmark("module_parallel/parallel_vector_sum")
utl_add_benchmark("module_parallel/thread_pool_comparison")
# utl_add_benchmark("module_profiler/profiling_overhead")
utl_add_benchmark("module_random/normal_distributions")
utl_add_benchmark("module_random/prngs")
utl_add_benchmark("module_random/uniform_int_distributions")
utl_add_benchmark("module_random/uniform_real_distributions")
