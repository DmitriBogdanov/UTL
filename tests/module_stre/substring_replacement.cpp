#include "tests/common.hpp"

#include "include/UTL/stre.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Substring replacement / Replace all") {
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "AA", "BBB") == "xxxBBBxxxBBBxxx");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "xx", "xxxx") == "xxxxxAAxxxxxAAxxxxx");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "A", "AAA") == "xxxAAAAAAxxxAAAAAAxxx");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "xA", "Ax") == "xxAxAxxAxAxxx");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "x", "") == "AAAA");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "xx", "") == "xAAxAAx");
    
    CHECK(stre::replace_all("xxxAAxxxAAxxx", "xxx", "") == "AAAA");

    CHECK(stre::replace_all("Some very very cool text ending with very", "very", "really") ==
          "Some really really cool text ending with really");

    CHECK(stre::replace_all("very short string", "super long replacement target",
                            "even longer replacement string just to test thing out") == "very short string");
}

TEST_CASE("Substring replacement / Replace first") {
    CHECK(stre::replace_first("xxxAAxxxAAxxx", "AA", "BBB") == "xxxBBBxxxAAxxx");
    
    CHECK(stre::replace_first("xxxAAxxxAAxxx", "xxx", "") == "AAxxxAAxxx");
    
    CHECK(stre::replace_first("xxxAAxxxAAxxx", "x", "xxx") == "xxxxxAAxxxAAxxx");

    CHECK(stre::replace_first("Some very very cool text ending with very", "very", "really") ==
          "Some really very cool text ending with very");

    CHECK(stre::replace_first("very short string", "super long replacement target",
                              "even longer replacement string just to test thing out") == "very short string");
}

TEST_CASE("Substring replacement / Replace last") {
    CHECK(stre::replace_last("xxxAAxxxAAxxx", "AA", "BBB") == "xxxAAxxxBBBxxx");
    
    CHECK(stre::replace_last("xxxAAxxxAAxxx", "xxx", "") == "xxxAAxxxAA");
    
    CHECK(stre::replace_last("xxxAAxxxAAxxx", "x", "xxx") == "xxxAAxxxAAxxxxx");

    CHECK(stre::replace_last("Some very very cool text ending with very", "very", "really") ==
          "Some very very cool text ending with really");

    CHECK(stre::replace_last("very short string", "super long replacement target",
                             "even longer replacement string just to test thing out") == "very short string");
}

TEST_CASE("Substring replacement / Replace prefix") {
    CHECK(stre::replace_prefix("xxxAAxxxAAxxx", "xxxA", "yyCC") == "yyCCAxxxAAxxx");

    CHECK(stre::replace_prefix("xxxAAxxxAAxxx", "AA", "CC") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_prefix("xxxAAxxxAAxxx", "AAxxx", "CC") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_prefix("xxxAAxxxAAxxx", "xxA", "yyCC") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_prefix("Some very very cool text ending with very", "Some very", "Any really") ==
          "Any really very cool text ending with very");

    CHECK(stre::replace_prefix("very short string", "super long replacement target",
                               "even longer replacement string just to test thing out") == "very short string");
}

TEST_CASE("Substring replacement / Replace suffix") {
    CHECK(stre::replace_suffix("xxxAAxxxAAxxx", "Axxx", "CCyy") == "xxxAAxxxACCyy");

    CHECK(stre::replace_suffix("xxxAAxxxAAxxx", "AA", "CC") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_suffix("xxxAAxxxAAxxx", "xxxAA", "CC") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_prefix("xxxAAxxxAAxxx", "Axx", "CCyy") == "xxxAAxxxAAxxx");

    CHECK(stre::replace_suffix("Some very very cool text ending with very", "very", "really") ==
          "Some very very cool text ending with really");

    CHECK(stre::replace_suffix("very short string", "super long replacement target",
                               "even longer replacement string just to test thing out") == "very short string");
}