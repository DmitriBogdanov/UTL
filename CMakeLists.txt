cmake_minimum_required(VERSION 3.9.1...3.22.1)
# should also work with all newer versions, the range specified
# here is the based in the highest version tested in practice

project(
    UTL
    VERSION 2.0.0
    DESCRIPTION "Collection of self-contained utility libraries."
    HOMEPAGE_URL "https://github.com/DmitriBogdanov/UTL"
)

# Source
include_directories("include")

# Tests & Benchmarks
add_subdirectory("tests")
add_subdirectory("benchmarks")