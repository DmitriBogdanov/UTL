// __________________________________ CONTENTS ___________________________________
//
//    Common utils / includes / namespaces used for benchmarking.
//    Reduces benchmark boilerplate, should not be included anywhere else.
// _______________________________________________________________________________

// ___________________ BENCH FRAMEWORK  ___________________

#define ANKERL_NANOBENCH_IMPLEMENT
#include "benchmarks/thirdparty/nanobench.h"

// ____________________ STD INCLUDES  _____________________

#include <chrono>   // chrono_literals::
#include <iostream> // cout, endl

// ____________________ IMPLEMENTATION ____________________

// ==============
// --- Macros ---
// ==============

#define REPEAT(repeats_) for (int count_ = 0; count_ < repeats_; ++count_)
#define DO_NOT_OPTIMIZE_AWAY ankerl::nanobench::doNotOptimizeAway

// ================
// --- Printing ---
// ================

template <class... Args>
void println(Args... args) {
    (std::cout << ... << args) << std::endl;
}

// ==========================
// --- Benchmark function ---
// ==========================

inline ankerl::nanobench::Bench bench;

template <class Func>
void benchmark(const char* name, Func lambda) {
    bench.run(name, lambda);
}

template <class Func>
void benchmark(const std::string& name, Func lambda) {
    bench.run(name.c_str(), lambda);
}

// ==================
// --- Namespaces ---
// ==================

using namespace std::chrono_literals;

namespace utl {}
using namespace utl;
