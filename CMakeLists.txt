cmake_minimum_required(VERSION 3.21.0...3.22.1)
# should work for all newer versions, specified range is the based in the highest version tested in practice

project(
    UTL
    VERSION 3.0.0
    DESCRIPTION "Collection of self-contained header-only libraries for C++17."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/UTL"
)

# Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Benchmarks
add_subdirectory("benchmarks")

# Tests
include(CTest)
enable_testing() # should be called before adding 'tests/' directory

add_subdirectory("tests")
