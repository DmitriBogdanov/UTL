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
    ) # all examples double as tests
    
endfunction()

# Note: Unlike tests, examples don't use root as a working directory since they don't have a need
#       to read any pre-existing data, but might create temporary files themselves

# Examples
utl_add_example("module_bit/additional_bitflag_examples")
utl_add_example("module_bit/general_usage")
utl_add_example("module_bit/using_enum_bitflags")
utl_add_example("module_bit/working_with_individual_bits")
utl_add_example("module_enum_reflect/reflecting_an_enum")
utl_add_example("module_integral/heterogeneous_comparison")
utl_add_example("module_integral/integer_division")
utl_add_example("module_integral/integral_literals")
utl_add_example("module_integral/narrow_cast")
utl_add_example("module_integral/saturated_math")
utl_add_example("module_integral/sign_conversion")
utl_add_example("module_json/complex_structure_reflection")
utl_add_example("module_json/error_handling")
utl_add_example("module_json/formatting")
utl_add_example("module_json/getters")
utl_add_example("module_json/parse_serialize_json")
utl_add_example("module_json/setters_and_type_conversions")
utl_add_example("module_json/structure_reflection")
utl_add_example("module_log/logging_to_multiple_sinks")
utl_add_example("module_log/logging_to_the_terminal")
utl_add_example("module_log/printing_and_stringification")
utl_add_example("module_math/conversions")
utl_add_example("module_math/index_functions")
utl_add_example("module_math/summation_and_product")
utl_add_example("module_math/template_math_functions")
