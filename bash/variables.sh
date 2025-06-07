# __________________________________ CONTENTS ___________________________________
#
#    All the variables used by other scripts, edit this file to configure
#       - compiler used by CMake
#       - test flags
#       - build jobs
#       ...
# _______________________________________________________________________________

# ===================
# ---- Constants ----
# ===================

path_include="include/UTL/*.hpp"
path_single_include="single_include/UTL.hpp"

directory_build="build/"
directory_tests="${directory_build}tests/"

script_create_single_header="bash/create_single_header.sh"
script_run_static_analysis="bash/run_static_analysis.sh"

cppcheck_suppressions_file=".cppcheck"
cppcheck_cache_directory=".cache-cppcheck"

# =======================
# ---- Configuration ----
# =======================

compiler="g++" # g++ / clang++-14
test_flags="--rerun-failed --output-on-failure --timeout 60"
coverage_flags="-T coverage"
build_jobs="6"
