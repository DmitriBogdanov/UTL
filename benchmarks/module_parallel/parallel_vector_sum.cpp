#include "benchmarks/common.hpp"

#include "include/UTL/parallel.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
// None

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

void benchmark_parallel_repeated_matmul(std::size_t threads, std::size_t N) {
    // Having workload divisible by a number of threads simplifies logic for some benchmarks
    if (N % threads != 0) throw std::runtime_error("Inappropriate workload size.");

    const std::vector<double> A(N, 1.);

    const auto partial_sum = [&](std::size_t low, std::size_t high) -> double {
        double sum = 0;
        for (std::size_t i = low; i < high; ++i) sum += A[i];
        return sum;
    };

    const auto thread_partial_sum = [&](std::size_t thread_num) -> double {
        const std::size_t low  = thread_num * (N / threads);
        const std::size_t high = (thread_num + 1) * (N / threads);
        return partial_sum(low, high);
    };

    println("\n\n====== BENCHMARKING ON: Parallel vector sum ======\n");
    println("Threads           -> ", threads);
    println("N                 -> ", N);
    println("Data memory usage -> ", N * sizeof(double) / 1e6, " MB");

    bench.timeUnit(1ms, "ms").minEpochIterations(5).warmup(10).relative(true); // global options

    bench.title("Parallel vector sum for " + std::to_string(threads) + " threads");

    // Serial sum (reference)
    double sum_serial{};
    benchmark("Serial version", [&]() {
        sum_serial = 0;
        for (const auto& e : A) sum_serial += e;
    });

    // Serial 'parallel::reduce()' sum with loop unrolling
    double sum_serial_reduce_unrolled{};
    parallel::set_thread_count(1);
    benchmark("parallel::reduce<4>() (single thread)", [&]() {
        sum_serial_reduce_unrolled = 0;
        sum_serial_reduce_unrolled = parallel::reduce<4>(A, parallel::sum<>{});
    });

    // Naive 'std::async()' sum
    double sum_async{};
    benchmark("Naive std::async()", [&]() {
        sum_async = 0;
        std::vector<std::future<double>> futures;
        for (std::size_t k = 0; k < threads; ++k) futures.emplace_back(std::async(thread_partial_sum, k));
        for (auto& future : futures) sum_async += future.get();
    });

    // 'parallel::reduce()' sum
    double sum_parallel_reduce{};
    parallel::set_thread_count(threads);
    benchmark("parallel::reduce()", [&]() {
        sum_parallel_reduce = 0;
        sum_parallel_reduce = parallel::reduce(A, parallel::sum<>{});
    });

    // 'parallel::reduce()' sum with loop unrolling
    double sum_parallel_reduce_unrolled{};
    parallel::set_thread_count(threads);
    benchmark("parallel::reduce<4>()", [&]() {
        sum_parallel_reduce_unrolled = 0;
        sum_parallel_reduce_unrolled = parallel::reduce<4>(A, parallel::sum<>{});
    });

    // OpenMP reduce
#ifdef BENCHMARK_OMP
    double sum_omp;
    omp_set_num_threads(thread_count);
    benchmark("OpenMP reduce", [&]() {
        sum_omp = 0;
#pragma omp parallel for reduction(+ : sum_omp)
        for (std::size_t i = 0; i < A.size(); ++i) sum_omp += A[i];
    });
#endif

    // Print sums to check correctness
    println("sum_serial                   -> ", std::setprecision(12), sum_serial);
    println("sum_serial_reduce_unrolled   -> ", std::setprecision(12), sum_serial_reduce_unrolled);
    println("sum_async                    -> ", std::setprecision(12), sum_async);
    println("sum_parallel_reduce          -> ", std::setprecision(12), sum_parallel_reduce);
    println("sum_parallel_reduce_unrolled -> ", std::setprecision(12), sum_parallel_reduce_unrolled);
#ifdef BENCHMARK_OMP
    println("sum_omp                      -> ", std::setprecision(12), sum_omp);
#endif
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    benchmark_parallel_repeated_matmul(2, 6'000'000);
    benchmark_parallel_repeated_matmul(4, 6'000'000);
    benchmark_parallel_repeated_matmul(6, 6'000'000);
    benchmark_parallel_repeated_matmul(2, 60'000'000);
    benchmark_parallel_repeated_matmul(4, 60'000'000);
    benchmark_parallel_repeated_matmul(6, 60'000'000);
}