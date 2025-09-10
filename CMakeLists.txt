cmake_minimum_required(VERSION 3.21.0...3.22.1)
# should work for all newer versions, specified range is the based in the highest version tested in practice

project(
    UTL
    VERSION 6.2.1
    DESCRIPTION "Collection of self-contained header-only libraries for C++17."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/UTL"
)

# Declare the library
add_library(utl_lib_include        INTERFACE)
add_library(utl_lib_single_include INTERFACE)

target_include_directories(utl_lib_include        INTERFACE "${PROJECT_SOURCE_DIR}/include"       )
target_include_directories(utl_lib_single_include INTERFACE "${PROJECT_SOURCE_DIR}/single_include")

target_compile_features(utl_lib_include        INTERFACE cxx_std_17)
target_compile_features(utl_lib_single_include INTERFACE cxx_std_17)

add_library(UTL::include        ALIAS utl_lib_include       )
add_library(UTL::single_include ALIAS utl_lib_single_include)

# Build benchmarks / examples / tests if project is top-level
if("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")

    # Benchmarks
    add_subdirectory("benchmarks")
    
    # Tests & examples
    include(CTest)
    enable_testing() # should be called before adding directories with tests
    
    add_subdirectory("tests")
    add_subdirectory("examples")
    add_subdirectory("auxiliary") # generating data & figures for documentation
    
endif()
