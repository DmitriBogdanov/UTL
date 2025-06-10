// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "test.hpp"

#include "UTL/shell.hpp"

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ===========================
// --- run_command() tests ---
// ===========================

// We assume Linux using bash or its derivatives as a default shell,
// this also tests temporary files as they are necessary for command to function

#if defined(linux) || defined(__linux__) || defined(__linux)

TEST_CASE("Capturing command stdout") {
    const auto res = shell::run_command("echo TEXT");

    CHECK(res.status == 0);
    CHECK(res.out == "TEXT");
    CHECK(res.err == "");
}

TEST_CASE("Capturing command stderr") {
    const auto res = shell::run_command("(echo TEXT >&2)");
    // 'echo' that redirects stdout to stderr so we can test it,
    // parentheses wrap command in a subshell so it can print to stderr without affecting the subsequent pipe

    CHECK(res.status == 0);
    CHECK(res.out == "");
    CHECK(res.err == "TEXT");
}

#endif