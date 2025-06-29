#include "benchmarks/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/random.hpp"

// Libraries to benchmarks against
// None

// Standard headers
#include <future>    // async(), future<>
#include <stdexcept> // runtime_error
#include <vector>    // vector<>

// ____________________ IMPLEMENTATION ____________________

// #define BENCHMARK_OMP // uncomment to benchmark against OpenMP

// Note: Benchmarking against OpenMP requires adding '-fopenmp' flag and linking 'OpenMP::OpenMP_CXX'

#ifdef BENCHMARK_OMP
#include <omp.h>
#endif

// =================
// --- Benchmark ---
// =================

// Benchmark for a repeated multiplication of 2 random square matrices,
// repeats showcase the benefits of threadpool thread reuse

void benchmark_parallel_repeated_matmul(std::size_t threads, std::size_t N) {
    // Having workload divisible by a number of threads simplifies logic for some benchmarks
    if (N % threads != 0) throw std::runtime_error("Inappropriate workload size.");

    constexpr int repeats = 20;

    // Prepare matrices
    std::vector<double> A(N * N, 0);
    std::vector<double> B(N * N, 0);
    std::vector<double> C(N * N, 0);
    for (auto& e : A) e = random::uniform_double();
    for (auto& e : B) e = random::uniform_double();

    const auto idx = [&](std::size_t i, std::size_t j) { return i * N + j; }; // 2D indexation

    // Matrix multiplication
    const auto compute_rows = [&](std::size_t low, std::size_t high) {
        for (std::size_t i = low; i < high; ++i)
            for (std::size_t k = 0; k < N; ++k)
                for (std::size_t j = 0; j < N; ++j) C[idx(i, j)] += A[idx(i, k)] * B[idx(k, j)];
    };

    const auto thread_compute_rows = [&](std::size_t thread_num) {
        const std::size_t low  = thread_num * (N / threads);
        const std::size_t high = (thread_num + 1) * (N / threads);
        compute_rows(low, high);
    };

    // Format header
    println("\n\n====== BENCHMARKING ON: Repeated matrix multiplication ======\n");
    println("Threads           -> ", threads);
    println("N                 -> ", N);
    println("repeats           -> ", repeats);
    println("Data memory usage -> ", N * N * 3 * sizeof(double) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(3).warmup(2).relative(true); // global options

    // Benchmark
    bench.title("Parallel repeated matmul for " + std::to_string(threads) + " threads");

    // Serial matmul (reference)
    benchmark("Serial version", [&]() {
        C = A;
        REPEAT(repeats)
        for (std::size_t k = 0; k < threads; ++k) thread_compute_rows(k);
        DO_NOT_OPTIMIZE_AWAY(C);
    });

    // Naive 'std::async()' matmul
    benchmark("Naive std::async()", [&]() {
        C = A;
        REPEAT(repeats) {
            std::vector<std::future<void>> futures;
            for (std::size_t k = 0; k < threads; ++k) futures.emplace_back(std::async(thread_compute_rows, k));
            for (const auto& future : futures) future.wait();
        }
        DO_NOT_OPTIMIZE_AWAY(C);
    });

    // 'parallel::task()' matmul
    parallel::set_thread_count(threads);
    benchmark("parallel::task()", [&]() {
        C = A;
        REPEAT(repeats) {
            for (std::size_t k = 0; k < threads; ++k) parallel::task(thread_compute_rows, k);
            parallel::wait_for_tasks();
        }
        DO_NOT_OPTIMIZE_AWAY(C);
    });

    // 'parallel::for_loop()' matmul
    parallel::set_thread_count(threads);
    benchmark("parallel::for_loop()", [&]() {
        C = A;
        REPEAT(repeats) { parallel::for_loop(parallel::IndexRange<std::size_t>{0, N}, compute_rows); }
        DO_NOT_OPTIMIZE_AWAY(C);
    });
    
    // OpenMP parallel for
#ifdef BENCHMARK_OMP
    omp_set_num_threads(threads);
    benchmark("OpenMP parallel for", [&]() {
        C = A;
        REPEAT(repeats) {
#pragma omp parallel for
            for (std::size_t k = 0; k < threads; ++k) thread_compute_rows(i);
        }
        DO_NOT_OPTIMIZE_AWAY(C);
    });
    const auto sum_omp = C.sum();
#endif
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    benchmark_parallel_repeated_matmul(3, 150);
    benchmark_parallel_repeated_matmul(5, 150);
    benchmark_parallel_repeated_matmul(6, 150);
    benchmark_parallel_repeated_matmul(3, 300);
    benchmark_parallel_repeated_matmul(5, 300);
    benchmark_parallel_repeated_matmul(6, 300);
}