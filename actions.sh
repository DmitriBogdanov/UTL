# __________________________________ CONTENTS ___________________________________
#   
#   This script contains shortcuts for building, running
#   and testing the project. All action keywords can be
#   chained which causes them to be executed one after another.
#
#   See "docs/guide_building_project.md" for the whole building guide.
#   
# ____________________________________ GUIDE ____________________________________
#   
#   Usage format:
#     > bash actions.sh [ACTIONS]
#   
#   Actions:
#     clear   - Clears "build/" folder
#     config  - Configures CMake with appropriate args
#     build   - Builds the project (requires configured CMake)
#     test    - Runs CTest tests (requires successful build)
#     profile - Runs currently selected executable with Callgrind
#   
#   Usage example:
#     > bash actions.sh clear config build test  
# _______________________________________________________________________________

# =======================
# ------ Functions ------
# =======================

source bash/variables.sh
source bash/functions.sh

command_clear() {
    if [ -d "$directory_build" ]; then
        rm --recursive $directory_build
        echo "Cleared directory [ $directory_build ]."
    else
        echo "Directory [ $directory_build ] is clear."
    fi
}

command_config() {
    require_command_exists "cmake"
    require_command_exists "$compiler"
    cmake -D CMAKE_CXX_COMPILER=$compiler -B $directory_build -S .
}

command_build() {
    # Invoke script to merge headers for single-include
    if [ -f "$script_create_single_header" ]; then
        printf "${ansi_green}Merging single header include...${ansi_reset}\n"
        bash "$script_create_single_header"
        printf "${ansi_green}Merge complete.${ansi_reset}\n"
    else
        printf "${ansi_red}# Error: Could not find \"$script_create_single_header\".${ansi_reset}\n"
    fi
    
    # Run CMake build
    require_command_exists "cmake"
    cmake --build $directory_build --parallel $build_jobs
}

command_test() {
    require_command_exists "ctest"
    cd $directory_tests
    ctest $test_flags
    cd ..
}

command_check() {
    # Invoke script to run static analyzers
    if [ -f "$script_run_static_analysis" ]; then
        printf "${ansi_green}Running static analyzers...${ansi_reset}\n"
        bash "$script_run_static_analysis"
        printf "${ansi_green}Analysis complete.${ansi_reset}\n"
    else
        printf "${ansi_red}# Error: Could not find \"$script_run_static_analysis\".${ansi_reset}\n"
    fi
}

# command_profile() {
#     require_command_exists "valgrind"
#     require_command_exists "callgrind_annotate"
#     require_command_exists "kcachegrind"
#     valgrind --tool=callgrind --dump-line=yes --callgrind-out-file="${directory_temp}callgrind.latest" ./$path_executable
#     callgrind_annotate --auto=yes --include="source/" "${directory_temp}callgrind.latest" > "${directory_temp}callgrind.annotate.txt"
#     kcachegrind "./${directory_temp}callgrind.latest"
# }

# =======================
# --- Action selector ---
# =======================

valid_command=false

for var in "$@"
do
    valid_command=false
    
    if [ "$var" = "clear" ]; then
        printf "${ansi_purple}# Action: Clear Files${ansi_reset}\n"
        command_clear
        valid_command=true
    fi

    if [ "$var" = "config" ]; then
        printf "${ansi_purple}# Action: CMake Configure${ansi_reset}\n"
        command_config
        valid_command=true
    fi

    if [ "$var" = "build" ]; then
        printf "${ansi_purple}# Action: CMake Build${ansi_reset}\n"
        command_build
        valid_command=true
    fi
    
    if [ "$var" = "test" ]; then
        printf "${ansi_purple}# Action: CMake Test${ansi_reset}\n"
        command_test
        valid_command=true
    fi
    
    if [ "$var" = "check" ]; then
        printf "${ansi_purple}# Action: Run Static Analysis${ansi_reset}\n"
        command_check
        valid_command=true
    fi
    
    if [ $valid_command = false ]; then
        printf "${ansi_red}# Error: Invalid action name -> ${var}${ansi_reset}\n"
        break
    fi

done