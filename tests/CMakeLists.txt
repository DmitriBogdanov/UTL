# Function for defining a test with proper args
function(utl_add_test ARG_REQUIRED_STANDARD ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "ADDITIONAL_CPP") # optional named argument

    string(REPLACE "/" "-" LOC_TARGET "${ARG_NAME}") # CMake doesn't like target names with slashes
    string(PREPEND LOC_TARGET "test-")               # so we don't intersect names with benchmarks & examples
    
    add_executable(${LOC_TARGET} ${ARG_NAME}.cpp ${ARG_ADDITIONAL_CPP})
    
    target_compile_features   (${LOC_TARGET} PRIVATE ${ARG_REQUIRED_STANDARD})
    target_compile_options    (${LOC_TARGET} PRIVATE ${UTL_TEST_FLAGS}       )
    target_link_options       (${LOC_TARGET} PRIVATE ${UTL_TEST_LINKER_FLAGS})
    target_include_directories(${LOC_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}     )
    
    add_test(
        NAME ${LOC_TARGET}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${LOC_TARGET} --no-intro --no-path-filenames --force-colors# --success
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
endfunction()

# Note 1: We want relative path to start at the top-level project directory '/' so we can access 'tests/data',
#         by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
#
# Note 2: '--force-colors' makes doctest show colored output in the terminal, but makes up CTest logs save ANSI
#         color codes. Without this flag doctest suppresses color because it detects CTest writing output to the file.

# Module tests
utl_add_test( cxx_std_17 "module_assertion/assertions"               )
utl_add_test( cxx_std_17 "module_bit/enum_bitflags"                  )
utl_add_test( cxx_std_17 "module_bit/group_bit_operations"           )
utl_add_test( cxx_std_17 "module_bit/individual_bit_operations"      )
utl_add_test( cxx_std_17 "module_bit/utils"                          )
utl_add_test( cxx_std_17 "module_describe_struct/forwarding"         )
utl_add_test( cxx_std_17 "module_describe_struct/reflection"         )
utl_add_test( cxx_std_17 "module_enum_reflect/reflection"            )
utl_add_test( cxx_std_17 "module_integral/rounding_integer_division" )
utl_add_test( cxx_std_17 "module_integral/saturated_math"            )
utl_add_test( cxx_std_17 "module_json/conversions"                   )
utl_add_test( cxx_std_17 "module_json/object_node_api"               )
utl_add_test( cxx_std_17 "module_json/parser_accepts_valid"          )
utl_add_test( cxx_std_17 "module_json/parser_rejects_invalid"        )
utl_add_test( cxx_std_17 "module_json/reflection"                    )
utl_add_test( cxx_std_17 "module_math/functions"                     )
utl_add_test( cxx_std_17 "module_log/custom_sinks"                   )
utl_add_test( cxx_std_17 "module_log/stringifier"                    )
utl_add_test( cxx_std_17 "module_log/styling"                        )
utl_add_test( cxx_std_17 "module_mvl/experimental"                   )
utl_add_test( cxx_std_17 "module_parallel/fuzzing"                   )
utl_add_test( cxx_std_17 "module_parallel/parallel_for_container"    )
utl_add_test( cxx_std_17 "module_parallel/parallel_for_index_range"  )
utl_add_test( cxx_std_17 "module_parallel/parallel_for_range"        )
utl_add_test( cxx_std_17 "module_parallel/parallel_reduce_container" )
utl_add_test( cxx_std_17 "module_parallel/parallel_reduce_range"     )
utl_add_test( cxx_std_17 "module_parallel/thread_pool_basics"        )
utl_add_test( cxx_std_17 "module_random/mean_min_max_sanity"         )
utl_add_test( cxx_std_17 "module_random/uniform_int_coverage"        )
utl_add_test( cxx_std_17 "module_random/uniform_int_range"           )
utl_add_test( cxx_std_17 "module_random/uniform_real_range"          )
utl_add_test( cxx_std_20 "module_reflect_struct/aggregates"          )
utl_add_test( cxx_std_17 "module_shell/shell_commands"               )
utl_add_test( cxx_std_17 "module_stre/case_conversions"              )
utl_add_test( cxx_std_17 "module_stre/character_classification"      )
utl_add_test( cxx_std_17 "module_stre/difference_measurement"        )
utl_add_test( cxx_std_17 "module_stre/padding"                       )
utl_add_test( cxx_std_17 "module_stre/repeating"                     )
utl_add_test( cxx_std_17 "module_stre/substring_checks"              )
utl_add_test( cxx_std_17 "module_stre/substring_replacement"         )
utl_add_test( cxx_std_17 "module_stre/tokenization"                  )
utl_add_test( cxx_std_17 "module_stre/trimming"                      )
utl_add_test( cxx_std_17 "module_strong_type/arithmetic"             )
utl_add_test( cxx_std_17 "module_strong_type/unique"                 )
utl_add_test( cxx_std_17 "module_table/formats"                      )
utl_add_test( cxx_std_17 "module_time/unit_split"                    )
# General tests
utl_add_test( cxx_std_20 "general_odr/main" ADDITIONAL_CPP "general_odr/second.cpp" )
