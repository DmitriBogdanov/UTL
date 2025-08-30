# Function for defining an auxiliary target with proper args
function(utl_add_auxiliary file_name)
    string(REPLACE "/" "-" target_name "${file_name}") # CMake doesn't like target names with slashes
    string(PREPEND target_name "auxiliary-")           # so we don't intersect names with benchmarks & tests
    
    add_executable(${target_name} ${file_name}.cpp)
    
    target_compile_features   (${target_name} PRIVATE ${UTL_COMPILE_FEATURES}      )
    target_compile_options    (${target_name} PRIVATE ${UTL_BENCHMARK_FLAGS}       )
    target_link_options       (${target_name} PRIVATE ${UTL_BENCHMARK_LINKER_FLAGS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}          )
    
endfunction()

# Auxiliary executables
utl_add_auxiliary("approx_distributions/generate")
utl_add_auxiliary("seed_correlation/generate")
utl_add_auxiliary("spectral_test/generate")
