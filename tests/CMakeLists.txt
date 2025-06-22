# Macro for defining a test with proper args
macro(utl_add_test filename)
    add_executable(${filename} ${filename}.cpp)
    
    target_compile_features(${filename} PRIVATE ${UTL_COMPILE_FEATURES} )
    target_compile_options (${filename} PRIVATE ${UTL_TEST_FLAGS}       )
    target_link_options    (${filename} PRIVATE ${UTL_TEST_LINKER_FLAGS})
    
    add_test(
        NAME ${filename}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${filename} --no-intro --no-path-filenames --force-colors
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # Note 1: We want relative path to start at the top-level project directory '/' so we can access 'tests/data',
    #         by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
    #
    # Note 2: '--force-colors' makes doctest show colored output in the terminal, but makes up CTest logs save ANSI
    #         color codes. Without this flag doctest suppresses color because it detects CTest writing output to the file.
    
endmacro()

# Tests
utl_add_test(test_bit)
utl_add_test(test_enum_reflect)
utl_add_test(test_integral)
utl_add_test(test_json)
utl_add_test(test_log)
utl_add_test(test_math)
utl_add_test(test_mvl)
utl_add_test(test_random)
utl_add_test(test_shell)
utl_add_test(test_stre)
utl_add_test(test_struct_reflect)
