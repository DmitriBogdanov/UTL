// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::parallel
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_parallel.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PARALLEL)
#ifndef UTLHEADERGUARD_PARALLEL
#define UTLHEADERGUARD_PARALLEL

#define UTL_PARALLEL_VERSION_MAJOR 1
#define UTL_PARALLEL_VERSION_MINOR 0
#define UTL_PARALLEL_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

#include <condition_variable> // condition_variable
#include <cstddef>            // size_t
#include <functional>         // bind()
#include <future>             // future<>, packaged_task<>
#include <mutex>              // mutex, recursive_mutex, lock_guard<>, unique_lock<>
#include <queue>              // queue<>
#include <thread>             // thread
#include <type_traits>        // decay_t<>, invoke_result_t<>
#include <utility>            // forward<>()
#include <vector>             // vector

// ____________________ DEVELOPER DOCS ____________________

// Library API resembles a minimalistic version of Intel TBB.
//
// In C++20 'std::jthread' can be used to simplify code a bit, no reason not to do so.
//
// In C++20 'unroll<>()' template can be improved to take index as a template-lambda-explicit-argument
// rather than a regular arg, ensuring its constexpr'ness. This may lead to a slight performance boost
// as current automatic unrolling doesn't propagate constexpr'ness of indexation as well.

// ____________________ IMPLEMENTATION ____________________

namespace utl::parallel::impl {

// =============
// --- Utils ---
// =============

template <class T, class Mutex = std::mutex>
class MutexProtected {
    T             value;
    mutable Mutex mutex;

public:
    MutexProtected() = default;
    MutexProtected(const T& value) : value(value), mutex() {}
    MutexProtected(T&& value) : value(std::move(value)), mutex() {}

    template <class Func>
    decltype(auto) apply(Func&& func) const {
        const std::lock_guard lock(this->mutex);
        return std::forward<Func>(func)(this->value);
    }

    template <class Func>
    decltype(auto) apply(Func&& func) {
        const std::lock_guard lock(this->mutex);
        return std::forward<Func>(func)(this->value);
    }

    [[nodiscard]] T&& release() { return std::move(this->value); }
};

[[nodiscard]] inline std::size_t max_thread_count() noexcept {
    const std::size_t detected_threads = std::thread::hardware_concurrency();
    return detected_threads ? detected_threads : 1;
    // 'hardware_concurrency()' returns '0' if it can't determine the number of threads,
    // in this case we can reasonably assume there is a single thread available
}

// No reason to include the entirety of <algorithm> just for 2 one-liner functions,
// so we implement 'std::size_t' min/max here
[[nodiscard]] constexpr std::size_t min_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? b : a; }
[[nodiscard]] constexpr std::size_t max_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? a : b; }

// Template for automatic loop unrolling.
//
// It is used in 'parallel::reduce()' to (optionally) speed up a tight loop while leaving the user
// with ability to easily control that unrolling. By default NO unrolling is used.
//
// Benchmarks indicate speedups ~130% to ~400% depending on CPU, compiler and options
// large unrolling (32) seems to be the best on benchmarks, however it may bloat the binary
// and take over too many branch predictor slots in case of min/max reductions, 4-8 seems
// like a reasonable sweet spot for most machines.
//
// One may think that it is a job of compiler to perform such optimizations, yet even with
// GCC '-Ofast -funroll-all-loop' and GCC unroll pragmas it fails to do them reliably.
//
// The reason it fails to do so is pretty clear for '-O2' and below - strictly speaking, most binary
// operations on floats are non-commutative (sum depends on the order of addition, for example), however
// since reduction is inherently not-order-preserving there is no harm in reordering operations some more
// and unrolling the loop so compiler will be able to use SIMD if it sees it as possible (which it often does).
//
// Why vectorization of simple loops still tends to fail with '-Ofast' which reduces conformance and
// allows reordering of math operations is unclear, but this is how it happens when actually measured.
//
template <class T, T... indices, class F>
constexpr void unroll_impl(std::integer_sequence<T, indices...>, F&& f) {
    (f(std::integral_constant<T, indices>{}), ...);
}
template <class T, T count, class F>
constexpr void unroll_expr(F&& f) {
    unroll_impl(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
}

// ===================
// --- Thread pool ---
// ===================

// A simple single-queue task threadpool, takes arbitrary callables as tasks,
// returns optional futures, supports pausing. Work stealing would probably be better
// in a general case, however it complicates the implementation quite noticeably and
// doesn't provide much measurable benefit under the API of this module.

// Note:
// We don't use 'MutexProtected' here to make implementation a bit more decoupled, plus such idiom isn't nearly as
// convenient once we enter the realm of non-trivial syncronization with recursive mutexes and condition variables.

class ThreadPool {
private:
    std::vector<std::thread>     threads;
    mutable std::recursive_mutex thread_mutex;

    std::queue<std::packaged_task<void()>> tasks{};
    mutable std::mutex                     task_mutex;

    std::condition_variable task_cv;          // used to notify changes to the task queue
    std::condition_variable task_finished_cv; // used to notify of finished tasks

    // Signals
    bool stopping = false; // signal for workers to shut down '.worker_main()'
    bool paused   = false; // signal for workers to not pull new tasks from the queue
    bool waiting  = false; // signal for workers that they should notify 'task_finished_cv' when
                           // finishing a task, which is used to implement 'wait for tasks' methods

    int tasks_running = 0; // number of tasks currently executed by workers

    // Main function for worker threads,
    // here workers wait for the queue, pull new tasks from it and run them
    void thread_main() {
        bool task_was_finished = false;

        while (true) {
            std::unique_lock<std::mutex> task_lock(this->task_mutex);

            if (task_was_finished) {
                --this->tasks_running;
                if (this->waiting) this->task_finished_cv.notify_all();
                // no need to set 'task_was_finished' back to 'false',
                // the only way we get back into this condition is if another task was finished
            }

            // Pool isn't destructing, isn't paused and there are tasks available in the queue
            //    => continue execution, a new task from the queue and start executing it
            // otherwise
            //    => unlock the mutex and wait until a new task is submitted,
            //       pool is unpaused or destruction is initiated
            this->task_cv.wait(task_lock, [&] { return this->stopping || (!this->paused && !this->tasks.empty()); });

            if (this->stopping) break; // escape hatch for thread destruction

            // Pull a new task from the queue and start executing it
            std::packaged_task<void()> task_to_execute = std::move(this->tasks.front());
            this->tasks.pop();
            ++this->tasks_running;
            task_lock.unlock();

            task_to_execute(); // NOTE: Should I catch exceptions here?
            task_was_finished = true;
        }
    }

    void start_threads(std::size_t worker_count_increase) {
        const std::lock_guard thread_lock(this->thread_mutex);
        // the mutex has to be recursive because we call '.start_threads()' inside '.set_num_threads()'
        // which also locks 'worker_mutex', if mutex wan't recursive we would deadlock trying to lock
        // it a 2nd time on the same thread.

        // NOTE: It feels like '.start_threads()' can be split into '.start_threads()' and
        // '.start_threads_assuming_locked()' which would remove the need for recursive mutex

        for (std::size_t i = 0; i < worker_count_increase; ++i)
            this->threads.emplace_back(&ThreadPool::thread_main, this);
    }

    void stop_all_threads() {
        const std::lock_guard thread_lock(this->thread_mutex);

        {
            const std::lock_guard task_lock(this->task_mutex);
            this->stopping = true;
            this->task_cv.notify_all();
        } // signals to all threads that they should stop running

        for (auto& worker : this->threads)
            if (worker.joinable()) worker.join();
        // 'joinable()' checks in needed so we don't try to join the master thread

        this->threads.clear();
    }

public:
    // --- Construction ---
    // --------------------

    ThreadPool() = default;

    explicit ThreadPool(std::size_t thread_count) { this->start_threads(thread_count); }

    ~ThreadPool() {
        this->unpause();
        this->wait_for_tasks();
        this->stop_all_threads();
    }

    // --- Threads ---
    // ---------------

    [[nodiscard]] std::size_t get_thread_count() const {
        const std::lock_guard thread_lock(this->thread_mutex);
        return this->threads.size();
    }

    void set_thread_count(std::size_t thread_count) {
        this->wait_for_tasks(); // all threads need to be free

        const std::size_t current_thread_count = this->get_thread_count();

        if (thread_count == current_thread_count) return;
        // 'quick escape' so we don't experience too much slowdown when the user calls '.set_thread_count()' repeatedly

        if (thread_count > current_thread_count) {
            this->start_threads(thread_count - current_thread_count);
        } else {
            this->stop_all_threads();
            {
                const std::lock_guard task_lock(this->task_mutex);
                this->stopping = false;
            }
            this->start_threads(thread_count);
            // It is possible to improve implementation by making the pool shrink by joining only the necessary amount
            // of threads instead of recreating the the whole pool, however that task is non-trivial and would likely
            // require a more granular signaling with one flag per thread instead of a global 'stopping' flag.
        }
    }

    // --- Task queue ---
    // ------------------

    template <class Func, class... Args>
    void add_task(Func&& func, Args&&... args) {
        const std::lock_guard task_lock(this->task_mutex);
        this->tasks.emplace(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        this->task_cv.notify_one(); // wakes up one thread (if possible) so it can pull the new task
    }

    template <class Func, class... Args,
              class FuncReturnType = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
    [[nodiscard]] std::future<FuncReturnType> add_task_with_future(Func&& func, Args&&... args) {
#if defined(_MSC_VER) || defined(_MSC_FULL_VER)
        // MSVC messed up implementation of 'std::packaged_task<>' so it is not movable (which,
        // according to the standard, it should be) and they can't fix it for 7 (and counting) years
        // because fixing the bug would change the ABI. See this thread about the bug report:
        // https://developercommunity.visualstudio.com/t/unable-to-move-stdpackaged-task-into-any-stl-conta/108672
        // As a workaround we wrap the packaged task into a shared pointer and add another layer of packaging.
        auto new_task = std::make_shared<std::packaged_task<FuncReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        this->add_task([new_task] { (*new_task)(); }); // horrible
        return new_task->get_future();
#else
        std::packaged_task<FuncReturnType()> new_task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        auto                                 future = new_task.get_future();
        this->add_task(std::move(new_task));
        return future;
#endif
    }

    void wait_for_tasks() {
        std::unique_lock task_lock(this->task_mutex);
        this->waiting = true;
        this->task_finished_cv.wait(task_lock, [&] { return this->tasks.empty() && this->tasks_running == 0; });
        this->waiting = false;
    }

    void clear_task_queue() {
        const std::lock_guard task_lock(this->task_mutex);
        this->tasks = {}; // for some reason 'std::queue' has no '.clear()', complexity O(N)
    }

    // --- Pausing ---
    // ---------------

    void pause() {
        const std::lock_guard task_lock(this->task_mutex);
        this->paused = true;
    }

    void unpause() {
        const std::lock_guard task_lock(this->task_mutex);
        this->paused = false;
        this->task_cv.notify_all();
    }

    [[nodiscard]] bool is_paused() const {
        const std::lock_guard task_lock(this->task_mutex);
        return this->paused;
    }
};

// =====================================
// --- Static thread pool operations ---
// =====================================

inline ThreadPool& static_thread_pool() {
    // no '[nodiscard]' since a call to this function might be used to initialize a threadpool
    static ThreadPool pool(max_thread_count());
    return pool;
}

[[nodiscard]] inline std::size_t get_thread_count() { return static_thread_pool().get_thread_count(); }

inline void set_thread_count(std::size_t thread_count) { static_thread_pool().set_thread_count(thread_count); }

// ================
// --- Task API ---
// ================

template <class Func, class... Args>
void task(Func&& func, Args&&... args) {
    static_thread_pool().add_task(std::forward<Func>(func), std::forward<Args>(args)...);
}

template <class Func, class... Args>
auto task_with_future(Func&& func, Args&&... args)
    -> std::future<std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>> {
    return static_thread_pool().add_task_with_future(std::forward<Func>(func), std::forward<Args>(args)...);
}

inline void wait_for_tasks() { static_thread_pool().wait_for_tasks(); }

// =======================
// --- Parallel ranges ---
// =======================

constexpr std::size_t default_grains_per_thread = 4;
// by default we distribute 4 tasks per thread, this number is purely empirical.
// We don't want to split up work into too many tasks (like with 'grain_size = 1')
// yet we want it to be a bit more granular than doing 1 task per thread since
// that would be horrible if tasks are noticeably uneven.

// Note:
// In range constructors we intentionally allow some possibly narrowing conversions like 'it1 - it2' to 'size_t'
// for better compatibility with containers that can use large ints as their difference type

template <class Idx>
struct IndexRange {
    Idx         first;
    Idx         last;
    std::size_t grain_size;

    IndexRange() = delete;
    constexpr IndexRange(Idx first, Idx last, std::size_t grain_size)
        : first(first), last(last), grain_size(grain_size) {}
    IndexRange(Idx first, Idx last)
        : IndexRange(first, last, max_size(1, (last - first) / (get_thread_count() * default_grains_per_thread))){};
};

template <class Iter>
struct Range {
    Iter        begin;
    Iter        end;
    std::size_t grain_size;

    Range() = delete;
    constexpr Range(Iter begin, Iter end, std::size_t grain_size) : begin(begin), end(end), grain_size(grain_size) {}
    Range(Iter begin, Iter end)
        : Range(begin, end, max_size(1, (end - begin) / (get_thread_count() * default_grains_per_thread))) {}


    template <class Container>
    Range(const Container& container) : Range(container.begin(), container.end()) {}

    template <class Container>
    Range(Container& container) : Range(container.begin(), container.end()) {}
}; // requires random-access iterator, but no good way to express that before C++20 concepts

// User-defined deduction guides
//
// By default, template constructors cannot deduce template argument 'Iter',
// however it is possible to define a custom deduction guide and achieve what we want
//
// See: https://en.cppreference.com/w/cpp/language/class_template_argument_deduction#User-defined_deduction_guides
template <class Container>
Range(const Container& container) -> Range<typename Container::const_iterator>;

template <class Container>
Range(Container& container) -> Range<typename Container::iterator>;

// ==========================
// --- 'Parallel for' API ---
// ==========================

template <class Idx, class Func>
void for_loop(IndexRange<Idx> range, Func&& func) {
    for (Idx i = range.first; i < range.last; i += range.grain_size)
        task(std::forward<Func>(func), i, min_size(i + range.grain_size, range.last));

    wait_for_tasks();
}

template <class Iter, class Func>
void for_loop(Range<Iter> range, Func&& func) {
    for (Iter i = range.begin; i < range.end; i += range.grain_size)
        task(std::forward<Func>(func), i, i + min_size(range.grain_size, range.end - i));

    wait_for_tasks();
}

template <class Container, class Func>
void for_loop(Container&& container, Func&& func) {
    for_loop(Range{std::forward<Container>(container)}, std::forward<Func>(func));
}

// =============================
// --- 'Parallel reduce' API ---
// =============================

constexpr std::size_t default_unroll = 1;

template <std::size_t unroll = default_unroll, class BinaryOp, class Iter, class T = typename Iter::value_type>
auto reduce(Range<Iter> range, BinaryOp&& op) -> T {

    MutexProtected<T> result = *range.begin;
    // we have to start from the 1st element and not 'T{}' because there is no guarantee
    // than doing so would be correct for some non-trivial 'T' and 'op'

    for_loop(Range<Iter>{range.begin + 1, range.end, range.grain_size}, [&](Iter low, Iter high) {
        const std::size_t range_size = high - low;

        // Execute unrolled loop if unrolling is enabled and the range is sufficiently large
        if constexpr (unroll > 1)
            if (range_size > unroll) {
                // (parallel section) Compute partial result (unrolled for SIMD)
                // Reduce unrollable part
                std::array<T, unroll> partial_results;
                unroll_expr<std::size_t, unroll>([&](std::size_t j) { partial_results[j] = *(low + j); });
                Iter it = low + unroll;
                for (; it < high - unroll; it += unroll)
                    unroll_expr<std::size_t, unroll>(
                        [&, it](std::size_t j) { partial_results[j] = op(partial_results[j], *(it + j)); });
                // Reduce remaining elements
                for (; it < high; ++it) partial_results[0] = op(partial_results[0], *it);
                // Collect the result
                for (std::size_t i = 1; i < partial_results.size(); ++i)
                    partial_results[0] = op(partial_results[0], partial_results[i]);

                // (critical section) Add partial result to the global one
                result.apply([&](auto&& res) { res = op(std::forward<decltype(res)>(res), partial_results[0]); });

                return; // skip the non-unrolled version
            }

        // Fallback onto a regular reduction loop otherwise
        // (parallel section) Compute partial result
        T partial_result = *low;
        for (auto it = low + 1; it != high; ++it) partial_result = op(partial_result, *it);

        // (critical section) Add partial result to the global one
        result.apply([&](auto&& res) { res = op(std::forward<decltype(res)>(res), partial_result); });
    });

    // Note 1:
    // We could also collect results into an array of partial results and then reduce it on the
    // main thread at the end, but that leads to a much less clean implementation and doesn't
    // seem to be measurably faster.

    // Note 2:
    // 'if constexpr (unroll > 1)' ensures that unrolling logic will have no effect
    //  whatsoever on the non-unrolled version of the template, it will not even compile.

    return result.release();
}

template <std::size_t unroll = default_unroll, class BinaryOp, class Container>
auto reduce(Container&& container, BinaryOp&& op) -> typename std::decay_t<Container>::value_type {
    return reduce<unroll>(Range{std::forward<Container>(container)}, std::forward<BinaryOp>(op));
}

// --- Pre-defined binary ops ---
// ------------------------------

// Note 1:
// Defining binary operations as free-standing functions has a huge negative
// effect on parallel::reduce performance, for example on 4 threads:
//    binary op is 'sum'        => speedup ~90%-180%
//    binary op is 'struct sum' => speedup ~370%
// This is caused by failed inlining and seems to be the reason why standard library implements
// 'std::plus' as a functor class and not a free-standing function. I spent 2 hours of my life
// and 4 rewrites of 'parallel::reduce()' on this.

// Note 2:
// 'sum' & 'prod' can are just aliases for standard functors, 'min' & 'max' on the other hand
// require custom, implementation. Note the '<void>' specialization for transparent functors,
// see https://en.cppreference.com/w/cpp/utility/functional

template <class... Args>
using sum = std::plus<Args...>; // without variadic 'Args...' we wouldn't be able to alias 'std::plus<>'

template <class... Args>
using prod = std::multiplies<Args...>;

template <class T>
struct min {
    constexpr const T& operator()(const T& lhs, const T& rhs) const noexcept(noexcept((lhs < rhs) ? lhs : rhs)) {
        return (lhs < rhs) ? lhs : rhs;
    }
};

template <>
struct min<void> {
    template <class T1, class T2>
    constexpr auto operator()(T1&& lhs, T2&& rhs) const
        noexcept(noexcept(std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs)))
            -> decltype(std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs)) {
        return std::less<>{}(lhs, rhs) ? std::forward<T1>(lhs) : std::forward<T2>(rhs);
    }

    using is_transparent = std::less<>::is_transparent;
};

template <class T>
struct max {
    constexpr const T& operator()(const T& lhs, const T& rhs) const noexcept(noexcept((lhs < rhs) ? rhs : lhs)) {
        return (lhs < rhs) ? rhs : lhs;
    }
};

template <>
struct max<void> {
    template <class T1, class T2>
    constexpr auto operator()(T1&& lhs, T2&& rhs) const
        noexcept(noexcept(std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs)))
            -> decltype(std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs)) {
        return std::less<>{}(lhs, rhs) ? std::forward<T1>(rhs) : std::forward<T2>(lhs);
    }

    using is_transparent = std::less<>::is_transparent;
};

} // namespace utl::parallel::impl

// ______________________ PUBLIC API ______________________

namespace utl::parallel {

using impl::ThreadPool;

using impl::static_thread_pool;
using impl::get_thread_count;
using impl::set_thread_count;

using impl::Range;
using impl::IndexRange;

using impl::task;
using impl::task_with_future;
using impl::wait_for_tasks;
using impl::for_loop;
using impl::reduce;

using impl::sum;
using impl::prod;
using impl::min;
using impl::max;

} // namespace utl::parallel

#endif
#endif // module utl::parallel
