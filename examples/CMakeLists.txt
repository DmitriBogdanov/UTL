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
# TODO: Add 'module_log/' examples after the rewrite
utl_add_example("module_math/conversions")
utl_add_example("module_math/index_functions")
utl_add_example("module_math/summation_and_product")
utl_add_example("module_math/template_math_functions")
# TODO: Add 'module_mvl/' examples after the rewrite
utl_add_example("module_parallel/awaitable_parallel_loop_with_specific_grain_size")
utl_add_example("module_parallel/awaitable_tasks")
utl_add_example("module_parallel/detached_tasks")
utl_add_example("module_parallel/parallel_for_loop")
utl_add_example("module_parallel/recursive_tasks")
utl_add_example("module_parallel/reducing_over_a_binary_operation")
utl_add_example("module_parallel/thread_introspection")
utl_add_example("module_parallel/using_a_local_thread_pool")
utl_add_example("module_predef/compilation_summary")
utl_add_example("module_predef/conditional_compilation")
utl_add_example("module_predef/optimization_macros")
utl_add_example("module_profiler/custom_style_and_exporting_results_to_a_file")
utl_add_example("module_profiler/profiling_code_segment")
utl_add_example("module_profiler/profiling_detached_threads_and_uploading_results")
utl_add_example("module_profiler/profiling_parallel_section")
utl_add_example("module_profiler/profiling_recursion")
utl_add_example("module_progressbar/progress_bar_with_custom_style")
utl_add_example("module_progressbar/progress_bar")
utl_add_example("module_random/constexpr_random")
utl_add_example("module_random/getting_random_values")
utl_add_example("module_random/using_custom_prngs_with_random")
utl_add_example("module_shell/running_shell_commands")
utl_add_example("module_shell/working_with_temporary_files")
utl_add_example("module_sleep/comparing_sleep_precision")
utl_add_example("module_stre/converting_string_case")
utl_add_example("module_stre/other_utilities")
utl_add_example("module_stre/padding_strings")
utl_add_example("module_stre/substring_checks")
utl_add_example("module_stre/token_manipulation")
utl_add_example("module_stre/trimming_strings")
utl_add_example("module_struct_reflect/basic_reflection")
utl_add_example("module_struct_reflect/debug_printing_with_utl_log")
utl_add_example("module_struct_reflect/field_and_entry_views")
utl_add_example("module_struct_reflect/iterating_over_a_generic_tuple")
utl_add_example("module_struct_reflect/using_reflection_to_define_binary_operations")
# TODO: 'module_struct_reflect/'
# TODO: Add 'module_table/' examples after the rewrite
# TODO: 'module_time/'