# Function for defining a benchmark with proper args
function(utl_add_benchmark ARG_REQUIRED_STANDARD ARG_NAME)
    string(REPLACE "/" "-" LOC_TARGET "${ARG_NAME}") # CMake doesn't like target names with slashes
    string(PREPEND LOC_TARGET "benchmark-") # so we don't intersect names with tests & examples
    
    add_executable(${LOC_TARGET} ${ARG_NAME}.cpp)
    
    target_compile_features   (${LOC_TARGET} PRIVATE ${ARG_REQUIRED_STANDARD}     )
    target_compile_options    (${LOC_TARGET} PRIVATE ${UTL_BENCHMARK_FLAGS}       )
    target_link_options       (${LOC_TARGET} PRIVATE ${UTL_BENCHMARK_LINKER_FLAGS})
    target_include_directories(${LOC_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}          )
    
endfunction()

# Benchmarks
utl_add_benchmark( cxx_std_17 "generic/ankerl"                           )
utl_add_benchmark( cxx_std_17 "generic/pmr"                              )
utl_add_benchmark( cxx_std_17 "generic/unseq"                            )
utl_add_benchmark( cxx_std_17 "module_json/parse_serialize"              )
utl_add_benchmark( cxx_std_17 "module_log/logging_overhead"              )
utl_add_benchmark( cxx_std_17 "module_log/stringification"               )
utl_add_benchmark( cxx_std_17 "module_mvl/experimental"                  )
utl_add_benchmark( cxx_std_17 "module_parallel/thread_pool_comparison"   )
utl_add_benchmark( cxx_std_17 "module_random/normal_distributions"       )
utl_add_benchmark( cxx_std_17 "module_random/prngs"                      )
utl_add_benchmark( cxx_std_17 "module_random/uniform_int_distributions"  )
utl_add_benchmark( cxx_std_17 "module_random/uniform_real_distributions" )
