include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
macro(add_utl_test filename)
    add_executable(${filename} ${filename}.cpp)
    
    target_compile_features(${filename} PRIVATE cxx_std_17)
    
    target_compile_options(${filename} PRIVATE
        -O0
        -g3 
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
        -fsanitize=undefined,address,leak
        -Wall -Wextra -Wpedantic -Werror
        -fmax-errors=1
        --coverage
    )
    # '-O0  -fno-omit-frame-pointer'           => no optimization makes compilation a bit faster
    # '-g3'                                    => leave as much debug info as possible, use '-ggdb3'
    #                                             for GDB-specific format that has even more info
    # '-fno-omit-frame-pointer'                => doesn't matter with '-O0', prohibits optimization
    #                                             than makes stack traces harder to analyze
    # '-fno-sanitize-recover=all'              => sanitizers should stop execution (which fails the test)
    #                                             when  triggered, default behavior is to continue
    # '-fsanitize=undefined,address,leak'      => enable sanitizers
    # '-Wall -Wextra -Wpedantic -Werror'       => more warnings, treat warnings as errors
    # '-fmax-errors=1'                         => stop after the first error, prevents too much error spam
    # '--coverage'                             => add instrumentation for coverage analysis, for GCC works as
    #                                             an alias for '-fprofile-arcs -ftest-coverage' when compiling 
    #                                             and '-lgcov' when linking
    # see GCC docs: https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
    #               https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
    
    target_link_options(${filename} PRIVATE -fsanitize=undefined,address,leak --coverage)
    # sanitizers are libraries that should be linked, otherwise we would get "undefined reference to function" errors
    # same thing with lgcov coverage instrumentation
    
    add_test(
        NAME ${filename}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${filename} --no-intro --no-path-filenames --force-colors
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        # we want relative path to start at the top-level project directory '/',
        # by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
    )
    
    # Note:
    # '--force-colors' makes doctest show colored output in the terminal, but makes up CTest logs save ANSI color
    # codes. Without this flag doctest suppresses color because it detects CTest writing output to the file.
    
endmacro()

# Tests
add_utl_test(test_bit)
add_utl_test(test_enum_reflect)
add_utl_test(test_integral)
add_utl_test(test_json)
add_utl_test(test_log)
add_utl_test(test_math)
add_utl_test(test_mvl)
add_utl_test(test_random)
add_utl_test(test_stre)
add_utl_test(test_struct_reflect)