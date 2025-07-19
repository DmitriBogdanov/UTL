#include "tests/common.hpp"

#include "include/UTL/shell.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// We assume Linux using bash or its derivatives as a default shell,
// this also tests temporary files as they are necessary for 'run_command()' to function

#ifdef __linux__

TEST_CASE("Shell commands / Capture stdout") {
    const auto res = shell::run_command("echo TEXT");

    CHECK(res.status == 0);
    CHECK(res.out == "TEXT");
    CHECK(res.err == "");
}

TEST_CASE("Shell commands / Capture stderr") {
    const auto res = shell::run_command("(echo TEXT >&2)");
    // 'echo' that redirects stdout to stderr so we can test it,
    // parentheses wrap command in a subshell so it can print to stderr without affecting the subsequent pipe

    CHECK(res.status == 0);
    CHECK(res.out == "");
    CHECK(res.err == "TEXT");
}

#endif