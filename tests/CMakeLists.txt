# Function for defining a test with proper args
function(utl_add_test file_name)
    string(REPLACE "/" "-" target_name "${file_name}") # CMake doesn't like target names with slashes
    string(PREPEND target_name "test-") # so we don't intersect names with benchmarks & examples
    
    add_executable(${target_name} ${file_name}.cpp)
    
    target_compile_features   (${target_name} PRIVATE ${UTL_COMPILE_FEATURES} )
    target_compile_options    (${target_name} PRIVATE ${UTL_TEST_FLAGS}       )
    target_link_options       (${target_name} PRIVATE ${UTL_TEST_LINKER_FLAGS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}     )
    
    add_test(
        NAME ${target_name}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target_name} --no-intro --no-path-filenames --force-colors --success
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
endfunction()

# Note 1: We want relative path to start at the top-level project directory '/' so we can access 'tests/data',
#         by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
#
# Note 2: '--force-colors' makes doctest show colored output in the terminal, but makes up CTest logs save ANSI
#         color codes. Without this flag doctest suppresses color because it detects CTest writing output to the file.

# Tests
utl_add_test("module_bit/enum_bitflags")
utl_add_test("module_bit/group_bit_operations")
utl_add_test("module_bit/individual_bit_operations")
utl_add_test("module_bit/utils")
utl_add_test("module_enum_reflect/reflection")
utl_add_test("module_integral/rounding_integer_division")
utl_add_test("module_integral/saturated_math")
utl_add_test("module_json/conversions")
utl_add_test("module_json/object_node_api")
utl_add_test("module_json/parser_accepts_valid")
utl_add_test("module_json/parser_rejects_invalid")
utl_add_test("module_json/reflection")
utl_add_test("module_math/functions")
utl_add_test("module_log/custom_stringifier")
utl_add_test("module_log/stringifier")
utl_add_test("module_log/styling")
utl_add_test("module_mvl/experimental")
utl_add_test("module_parallel/fuzzing")
utl_add_test("module_parallel/parallel_for_container")
utl_add_test("module_parallel/parallel_for_index_range")
utl_add_test("module_parallel/parallel_for_range")
utl_add_test("module_parallel/parallel_reduce_container")
utl_add_test("module_parallel/parallel_reduce_range")
utl_add_test("module_parallel/thread_pool_basics")
utl_add_test("module_random/mean_min_max_sanity")
utl_add_test("module_random/uniform_int_coverage")
utl_add_test("module_random/uniform_int_range")
utl_add_test("module_random/uniform_real_range")
utl_add_test("module_shell/shell_commands")
utl_add_test("module_stre/case_conversions")
utl_add_test("module_stre/other_utils")
utl_add_test("module_stre/padding")
utl_add_test("module_stre/substring_checks")
utl_add_test("module_stre/token_manipulation")
utl_add_test("module_stre/trimming")
utl_add_test("module_struct_reflect/perfect_forwarding")
utl_add_test("module_struct_reflect/reflection")
utl_add_test("module_time/unit_split")
