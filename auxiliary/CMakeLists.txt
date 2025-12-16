# Function for defining an auxiliary target with proper args
function(utl_add_auxiliary ARG_REQUIRED_STANDARD ARG_NAME)
    string(REPLACE "/" "-" LOC_TARGET "${ARG_NAME}") # CMake doesn't like target names with slashes
    string(PREPEND LOC_TARGET "auxiliary-")           # so we don't intersect names with benchmarks & tests
    
    add_executable(${LOC_TARGET} ${ARG_NAME}.cpp)
    
    target_compile_features   (${LOC_TARGET} PRIVATE ${ARG_REQUIRED_STANDARD}     )
    target_compile_options    (${LOC_TARGET} PRIVATE ${UTL_BENCHMARK_FLAGS}       )
    target_link_options       (${LOC_TARGET} PRIVATE ${UTL_BENCHMARK_LINKER_FLAGS})
    target_include_directories(${LOC_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}          )
    
endfunction()

# Auxiliary executables
utl_add_auxiliary( cxx_std_17 "approx_distributions/generate" )
utl_add_auxiliary( cxx_std_17 "seed_correlation/generate"     )
utl_add_auxiliary( cxx_std_17 "spectral_test/generate"        )
