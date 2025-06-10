// __________ BENCHMARK FRAMEWORK & LIBRARY  __________

#include "benchmark.hpp"

#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

// _____________ BENCHMARK IMPLEMENTATION _____________

// #define BENCHMARK_OMP

#ifdef BENCHMARK_OMP
#include <omp.h>
#endif

// Benchmark for: repeated parallel matrix multiplication
//    for (repeats) C += A * B;
// assuming naive implementation.
//
// Repeats are necessary to showcase the difference between a thread pool that creates
// threads once and a naive async approach that doesn't keep the threads after execution.
//
// A real-world example of such workload would be an iterative method that
// performs matrix operations on every step.
//
// We use C.sum() to verify the result.
//
void benchmark_matrix_multiplication() {
    using namespace utl;

    constexpr std::size_t N            = 600; // should be divisible by 'worker_count'
    constexpr std::size_t thread_count = 4;
    constexpr int         repeats      = 20; // how many times to repeat matrix multiplication on each epoch

    const mvl::Matrix<double> A(N, N, [] { return random::uniform_double(); });
    const mvl::Matrix<double> B(N, N, [] { return random::uniform_double(); });
    mvl::Matrix<double>       C;

    log::println("\n\n====== BENCHMARKING ON: Repeated matrix multiplication ======\n");
    log::println("Threads           -> ", thread_count);
    log::println("N                 -> ", N);
    log::println("repeats           -> ", repeats);
    log::println("Data memory usage -> ", math::to_memory_units(N * N * 3 * sizeof(double)), " MiB");

    const auto compute_rows = [&](std::size_t low, std::size_t high) {
        for (std::size_t i = low; i < high; ++i)
            for (std::size_t k = 0; k < N; ++k)
                for (std::size_t j = 0; j < N; ++j) C(i, j) += A(i, k) * B(k, j);
    };

    const auto compute_rows_for_worker = [&](unsigned int worker_num) {
        const std::size_t low  = worker_num * (N / thread_count);
        const std::size_t high = (worker_num + 1) * (N / thread_count);
        compute_rows(low, high);
    };

    // Global benchmark options
    bench.minEpochIterations(6)
        .timeUnit(1ms, "ms")
        .title("Repeated matrix multiplication")
        .relative(true)
        .warmup(2);

    // Serial benchmark (reference)
    benchmark("Serial version", [&]() {
        C = A;
        REPEAT(repeats)
        for (std::size_t k = 0; k < thread_count; ++k) compute_rows_for_worker(k);
    });

    const auto sum_serial = C.sum();
    // we will use this sum to verify that other computations are in fact
    // doing the same work and not computing some bogus value

    // OpenMP parallel for
#ifdef BENCHMARK_OMP
    omp_set_num_threads(thread_count);
    benchmark("OpenMP parallel for", [&]() {
        C = A;
        REPEAT(repeats) {
#pragma omp parallel for
            for (std::size_t k = 0; k < thread_count; ++k) compute_rows_for_worker(k);
        }
    });
    const auto sum_omp = C.sum();
#endif

    // Naive std::async()
    benchmark("Naive std::async()", [&]() {
        C = A;
        REPEAT(repeats) {
            std::vector<std::future<void>> futures;
            for (std::size_t k = 0; k < thread_count; ++k) futures.emplace_back(std::async(compute_rows_for_worker, k));
            for (const auto& future : futures) future.wait();
        }
    });
    const auto sum_std_async = C.sum();

    // parallel::task();
    parallel::set_thread_count(thread_count);
    benchmark("parallel::task()", [&]() {
        C = A;
        REPEAT(repeats) {
            for (std::size_t k = 0; k < thread_count; ++k) parallel::task(compute_rows_for_worker, k);
            parallel::wait_for_tasks();
        }
    });
    const auto sum_parallel_task = C.sum();

    // parallel::for_loop();
    benchmark("parallel::for_loop()", [&]() {
        C = A;
        REPEAT(repeats) { parallel::for_loop(parallel::IndexRange<std::size_t>{0, N}, compute_rows); }
    });
    const auto sum_parallel_for_loop = C.sum();

    // Verify correctness
    log::println();
    table::create({40, 20});
    table::hline();
    table::cell("Method", "Control sum");
    table::hline();
    table::cell("Serial", sum_serial);
#ifdef BENCHMARK_OMP
    table::cell("OpenMP parallel for", sum_omp);
#endif
    table::cell("Naive std::async()", sum_std_async);
    table::cell("parallel::task()", sum_parallel_task);
    table::cell("parallel::for_loop()", sum_parallel_for_loop);
    // Notes:
    //
    // std::async() is extremely inconsistent.
    //
    // Threadpool seems to perform ~according to the sensible expectations.
}

// Benchmark for: parallel vector sum
//    for (repeats) C += A * B;
//
// We use control sum to verify the result.
//
void benchmark_sum() {
    constexpr std::size_t N            = 50'000'000; // should be divisible by 'worker_count'
    constexpr std::size_t thread_count = 4;
    
    log::println("\n\n====== BENCHMARKING ON: Parallel vector sum ======\n");
    log::println("Threads           -> ", thread_count);
    log::println("N                 -> ", N);
    log::println("Data memory usage -> ", math::to_memory_units(N * sizeof(double)), " MiB");

    const std::vector<double> A(N, 1.);

    const auto compute_partial_sum = [&](std::size_t low, std::size_t high) -> double {
        double s = 0;
        for (std::size_t i = low; i < high; ++i) s += A[i];
        return s;
    };

    const auto compute_partial_sum_for_worker = [&](std::size_t worker_num) -> double {
        const std::size_t low  = worker_num * (N / thread_count);
        const std::size_t high = (worker_num + 1) * (N / thread_count);
        return compute_partial_sum(low, high);
    };

    // Global benchmark options
    bench.minEpochIterations(10)
        .timeUnit(1ms, "ms")
        .title("Parallel vector sum")
        .relative(true)
        .warmup(10);

    // Serial benchmark (reference)
    double sum_serial;
    benchmark("Serial version", [&]() {
        sum_serial = 0;
        for (auto e : A) sum_serial += e;
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

    // Naive std::async()
    double sum_async;
    benchmark("Naive std::async()", [&]() {
        sum_async = 0;
        std::vector<std::future<double>> futures;
        for (std::size_t k = 0; k < thread_count; ++k)
            futures.emplace_back(std::async(compute_partial_sum_for_worker, k));
        for (auto& future : futures) sum_async += future.get();
    });

    // parallel::reduce()
    double sum_parallel_reduce;
    parallel::set_thread_count(thread_count);
    benchmark("parallel::reduce()", [&]() {
        sum_parallel_reduce = 0;
        sum_parallel_reduce = parallel::reduce(A, parallel::sum<>{});
    });
    
    // parallel::reduce()
    double sum_parallel_reduce_unrolled;
    parallel::set_thread_count(thread_count);
    benchmark("parallel::reduce<4>() (loop unrolling enabled)", [&]() {
        sum_parallel_reduce_unrolled = 0;
        sum_parallel_reduce_unrolled = parallel::reduce<4>(A, parallel::sum<>{});
    });
    
    // Verify correctness
    log::println();
    table::create({50, 20});
    table::set_formats({table::DEFAULT(), table::FIXED(10)});
    table::hline();
    table::cell("Method", "Control sum");
    table::hline();
    table::cell("Serial", sum_serial);
#ifdef BENCHMARK_OMP
    table::cell("OpenMP reduce", sum_omp);
#endif
    table::cell("Naive std::async", sum_async);
    table::cell("parallel::reduce()", sum_parallel_reduce);
    table::cell("parallel::reduce<4>() (loop unrolling enabled))", sum_parallel_reduce_unrolled);
}

int main() {
    benchmark_sum();
    //benchmark_matrix_multiplication();
}