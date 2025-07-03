# Function for defining a test with proper args
function(utl_add_example file_name)
    string(REPLACE "/" "-" target_name "${file_name}") # CMake doesn't like target names with slashes
    string(PREPEND target_name "example-")             # so we don't intersect names with benchmarks & tests
    
    add_executable(${target_name} ${file_name}.cpp)
    
    target_compile_features   (${target_name} PRIVATE ${UTL_COMPILE_FEATURES} )
    target_compile_options    (${target_name} PRIVATE ${UTL_TEST_FLAGS}       )
    target_link_options       (${target_name} PRIVATE ${UTL_TEST_LINKER_FLAGS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}     )
    
    add_test(
        NAME ${target_name}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target_name} --no-intro --no-path-filenames --force-colors
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    ) # all examples double as tests
    
endfunction()

# Examples
utl_add_example("module_bit/additional_bitflag_examples")
utl_add_example("module_bit/general_usage")
utl_add_example("module_bit/using_enum_bitflags")
utl_add_example("module_bit/working_with_individual_bits")
utl_add_example("module_enum_reflect/reflecting_an_enum")
