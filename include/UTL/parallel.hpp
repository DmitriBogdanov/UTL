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

#define UTL_PARALLEL_VERSION_MAJOR 2
#define UTL_PARALLEL_VERSION_MINOR 1
#define UTL_PARALLEL_VERSION_PATCH 1

// _______________________ INCLUDES _______________________

#include <condition_variable> // condition_variable
#include <cstdint>            // uint64_t
#include <functional>         // plus<>, multiplies<>, function<>
#include <future>             // future<>, promise<>
#include <memory>             // shared_ptr<>
#include <mutex>              // mutex, scoped_lock<>, unique_lock<>
#include <optional>           // optional<>, nullopt
#include <queue>              // queue<>, deque<>, size_t, ptrdiff_t
#include <stdexcept>          // current_exception, runtime_error
#include <thread>             // thread
#include <utility>            // forward<>()
#include <vector>             // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Work-stealing summary:
//
//    - We use several queues:
//         - All  threads have a global task queue
//         - Each thread  has  a local  task deque
//    - Tasks go into different queues depending on their source:
//         - Work queued from a     pool thread => recursive task, goes to the front of local  deque
//         - Work queued from a non-pool thread => external  task, goes to the back  of global queue
//    - When threads are looking for work they search in 3 steps:
//         1. Check local  deque,  work here can be popped from the front
//         2. Check other  deques, work here can be stolen from the back
//         3. Check global queue,  work here can be popped from the front
//    - To resolve recursive deadlocks we use a custom future:
//         - Recursive task calls '.wait()' on its future => pop / steal work from local deques until finished
//
// Ideally we would want to use a different algorithm with Chase-Lev lock-free local SPMC queues for work-stealing,
// and a global lock-free MPMC queue for outside tasks, however properly implementing such queues is a task of
// incredible complexity, which is further exacerbated by the fact that 'std::function' has a potentially throwing
// move-assignment, which disqualifies it from ~80% of existing lock-free queue implementations.
//
// Newer standards would enable several improvements in terms of implementation:
//    - In C++20 'std::jthread' can be used to simplify joining and add stop tokens
//    - In C++23 'std::move_only_function<>' can be used as a more efficient task type,
//      however this still doesn't resolve the issue of throwing move assignment
//    - In C++20 atomic wait / semaphores / barriers can be used to improve efficiency
//      of some syncronization
//
// It is also possible to add task priority with a bit of constexpr logic and possibly reduce locking
// in some parts of the scheduling, this is a work for future releases.

// ____________________ IMPLEMENTATION ____________________

namespace utl::parallel::impl {

// ============================
// --- Thread introspection ---
// ============================

namespace this_thread {

inline thread_local std::optional<std::size_t> worker_index    = std::nullopt;
inline thread_local std::optional<void*>       thread_pool_ptr = std::nullopt;

[[nodiscard]] inline std::optional<std::size_t> get_index() noexcept { return worker_index; }
[[nodiscard]] inline std::optional<void*>       get_pool() noexcept { return thread_pool_ptr; }

}; // namespace this_thread

[[nodiscard]] inline std::size_t hardware_concurrency() noexcept {
    const std::size_t     detected_count = std::thread::hardware_concurrency();
    constexpr std::size_t fallback_count = 4;
    return detected_count ? detected_count : fallback_count;
    // if 'hardware_concurrency()' struggles to determine the number of threads, we fallback onto a reasonable default
}

// ===================
// --- Thread pool ---
// ===================

class ThreadPool;

namespace ws_this_thread { // same this as thread introspection from public API, but more convenient for internal use
inline thread_local ThreadPool* thread_pool_ptr = nullptr;
inline thread_local std::size_t worker_index    = std::size_t(-1);
}; // namespace ws_this_thread

inline std::size_t splitmix64() noexcept {
    thread_local std::uint64_t state = ws_this_thread::worker_index;

    std::uint64_t result = (state += 0x9E3779B97f4A7C15);
    result               = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result               = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return static_cast<std::size_t>(result ^ (result >> 31));
} // very fast & simple PRNG

class ThreadPool {
    using task_type         = std::function<void()>;
    using global_queue_type = std::queue<task_type>;
    using local_queue_type  = std::deque<task_type>;

    std::vector<std::thread> workers;
    std::mutex               workers_mutex;

    global_queue_type global_queue;
    std::mutex        global_queue_mutex;

    std::vector<local_queue_type> local_queues;
    std::vector<std::mutex>       local_queues_mutexes;

    std::condition_variable task_available_cv;
    std::condition_variable task_done_cv;
    std::mutex              task_mutex;

    std::size_t tasks_running = 0; // protected by task mutex
    std::size_t tasks_pending = 0;

    bool waiting     = false; // protected by task mutex
    bool terminating = true;

private:
    void spawn_workers(std::size_t count) {
        this->workers              = std::vector<std::thread>(count);
        this->local_queues         = std::vector<local_queue_type>(count);
        this->local_queues_mutexes = std::vector<std::mutex>(count);
        {
            const std::scoped_lock task_lock(this->task_mutex);
            this->terminating = false;
        }
        for (std::size_t i = 0; i < count; ++i) this->workers[i] = std::thread([this, i] { this->worker_main(i); });
    }

    void terminate_workers() {
        {
            const std::scoped_lock polling_lock(this->task_mutex);
            this->terminating = true;
        }
        this->task_available_cv.notify_all();

        for (std::size_t i = 0; i < this->workers.size(); ++i)
            if (this->workers[i].joinable()) this->workers[i].join();
    }

    void worker_main(std::size_t worker_index) {
        this_thread::thread_pool_ptr    = this;
        this_thread::worker_index       = worker_index;
        ws_this_thread::thread_pool_ptr = this;
        ws_this_thread::worker_index    = worker_index;

        while (true) {
            std::unique_lock task_lock(this->task_mutex);

            // Wake up thread pool 'wait()' if necessary
            if (this->waiting && !this->tasks_pending && !this->tasks_running) this->task_done_cv.notify_all();

            // Tasks pending       => continue execution and pull tasks to execute
            // Pool is terminating => continue execution and break out of the main loop
            // otherwise           => wait
            this->task_available_cv.wait(task_lock, [this] { return this->terminating || this->tasks_pending; });

            // Terminate if necessary
            if (this->terminating) break;

            task_lock.unlock();

            // Try to pull in a task
            task_type task;
            if (this->try_pop_local(task) || this->try_steal(task) || this->try_pop_global(task)) {
                task_lock.lock();
                --this->tasks_pending;
                ++this->tasks_running;
                task_lock.unlock();

                task();

                task_lock.lock();
                --this->tasks_running;
                task_lock.unlock();
            }
        }

        this_thread::thread_pool_ptr    = std::nullopt;
        this_thread::worker_index       = std::nullopt;
        ws_this_thread::thread_pool_ptr = nullptr;
        ws_this_thread::worker_index    = std::size_t(-1);
    }

    bool try_pop_local(task_type& task) {
        const std::scoped_lock local_queue_lock(this->local_queues_mutexes[ws_this_thread::worker_index]);

        auto& local_queue = this->local_queues[ws_this_thread::worker_index];

        if (local_queue.empty()) return false;

        task = std::move(local_queue.front());
        local_queue.pop_front();
        return true;
    }

    bool try_steal(task_type& task) {
        for (std::size_t attempt = 0; attempt < this->workers.size(); ++attempt) {
            const std::size_t i = splitmix64() % this->workers.size();

            if (i == ws_this_thread::worker_index) continue; // don't steal from yourself

            const std::scoped_lock local_queue_lock(this->local_queues_mutexes[i]);

            auto& local_queue = this->local_queues[i];

            if (local_queue.empty()) continue;

            task = std::move(local_queue.back());
            local_queue.pop_back();
            return true;
        }

        return false;
    }

    bool try_pop_global(task_type& task) {
        const std::scoped_lock global_queue_lock(this->global_queue_mutex);

        if (this->global_queue.empty()) return false;

        task = std::move(this->global_queue.front());
        this->global_queue.pop();
        return true;
    }

public:
    explicit ThreadPool(std::size_t count = std::thread::hardware_concurrency()) { this->spawn_workers(count); }

    ~ThreadPool() noexcept {
        try {
            this->wait();
            this->terminate_workers();
        } catch (...) {} // no throwing from the destructor
    }

    template <class T = void>
    struct future_type {
        std::future<T> future;

        void fallthrough() const {
            ThreadPool* pool = ws_this_thread::thread_pool_ptr;
            
            if (!pool) return;
            
            // Execute recursive tasks from local queues until this future is ready
            task_type task;
            while (pool->try_pop_local(task) || pool->try_steal(task)) {
                pool->task_mutex.lock();
                --pool->tasks_pending;
                ++pool->tasks_running;
                pool->task_mutex.unlock();
            
                task();
            
                pool->task_mutex.lock();
                --pool->tasks_running;
                pool->task_mutex.unlock();
                
                if (this->is_ready()) return;
            }
        }
        
    bool is_ready() const {
        return this->future.wait_for(std::chrono::seconds{0}) == std::future_status::ready;
    }

    public:
        future_type(std::future<T>&& future) : future(std::move(future)) {} // conversion from regular future
        future_type& operator=(std::future<T>&& future) {
            this->future = std::move(future);
            return *this;
        }

        using is_recursive = void;

        auto get() {
            this->fallthrough();
            return this->future.get();
        }

        bool valid() const noexcept { return this->future.valid(); }

        void wait() const {
            this->fallthrough();
            this->future.wait();
        }

        template <class Rep, class Period>
        std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
            this->fallthrough();
            return this->future.wait_for(timeout_duration);
        }

        template <class Clock, class Duration>
        std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) const {
            this->fallthrough();
            return this->future.wait_until(timeout_time);
        }
    };

    void set_thread_count(std::size_t count) {
        if (ws_this_thread::thread_pool_ptr == this)
            throw std::runtime_error("Cannot resize thread pool from its own pool thread.");

        const std::scoped_lock workers_lock(this->workers_mutex);

        this->wait();
        this->terminate_workers();
        this->spawn_workers(count);
    }

    [[nodiscard]] std::size_t get_thread_count() {
        if (ws_this_thread::thread_pool_ptr == this) return this->workers.size();
        // calls from inside the pool shouldn't lock, otherwise we could deadlock the thread by trying to
        // query the thread count while the pool was instructed to resize, worker count is constant during
        // a worker lifetime so it's safe to query without a lock anyways

        const std::scoped_lock workers_lock(this->workers_mutex);

        return this->workers.size();
    }

    void wait() {
        std::unique_lock task_lock(this->task_mutex);

        this->waiting = true;
        this->task_done_cv.wait(task_lock, [this] { return !this->tasks_pending && !this->tasks_running; });
        this->waiting = false;
    }

    template <class F, class... Args>
    void detached_task(F&& f, Args&&... args) {
        auto closure = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // Recursive task
        if (ws_this_thread::thread_pool_ptr == this) {
            const std::scoped_lock local_queue_lock(this->local_queues_mutexes[ws_this_thread::worker_index]);
            this->local_queues[ws_this_thread::worker_index].push_front(std::move(closure));
        }
        // Regular task
        else {
            const std::scoped_lock global_queue_mutex(this->global_queue_mutex);
            this->global_queue.emplace(std::move(closure));
        }

        {
            const std::scoped_lock task_lock(this->task_mutex);
            ++this->tasks_pending;
        }
        this->task_available_cv.notify_one();
    }

    template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    future_type<R> awaitable_task(F&& f, Args&&... args) {
        auto closure = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        const std::shared_ptr<std::promise<R>> promise = std::make_shared<std::promise<R>>();
        // ideally we could just move promise into a lambda and use it as is, however that makes the lambda
        // non-copyable (since promise itself is move-only) which doesn't work with 'std::function<>', this
        // is fixed in C++23 with 'std::move_only_function<>'

        future_type<R> future = promise->get_future();

        this->detached_task([closure = std::move(closure), promise = std::move(promise)]() mutable {
            try {
                if constexpr (std::is_void_v<R>) {
                    closure();
                    promise->set_value();
                } else {
                    promise->set_value(closure());
                } // 'promise->set_value(f())' when 'f()' returns 'void' is a compile error, so we use a workaround
            } catch (...) {
                try {
                    promise->set_exception(std::current_exception()); // this may still throw
                } catch (...) {}
            }
        });

        return future;
    }
};

template <class T = void>
using Future = ThreadPool::future_type<T>;

// ==============
// --- Ranges ---
// ==============

// --- SFINAE helpers ---
// ----------------------

template <bool Cond>
using require = std::enable_if_t<Cond, bool>;

template <class T, class... Args>
using require_invocable = require<std::is_invocable_v<T, Args...>>;
// this is simple convenience

template <class T, class = void>
struct has_iter : std::false_type {};
template <class T>
struct has_iter<T, std::void_t<decltype(std::declval<typename T::iterator>())>> : std::true_type {};

template <class T, class = void>
struct has_const_iter : std::false_type {};
template <class T>
struct has_const_iter<T, std::void_t<decltype(std::declval<typename T::const_iterator>())>> : std::true_type {};

template <class T>
using require_has_some_iter = require<has_iter<T>::value || has_const_iter<T>::value>;
// needed to restrict template 'Range' constructor from a container, otherwise it takes priority over copy / move ctors

template <class T, class = void>
struct is_recursive : std::false_type {};
template <class T>
struct is_recursive<T, std::void_t<decltype(std::declval<typename T::is_recursive>())>> : std::true_type {};
template <class T>
constexpr bool is_recursive_v = is_recursive<T>::value;

// --- Utils ---
// -------------

[[nodiscard]] constexpr std::size_t min_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? b : a; }
[[nodiscard]] constexpr std::size_t max_size(std::size_t a, std::size_t b) noexcept { return (b < a) ? a : b; }
// no reason to include the entirety of <algorithm> just for 2 one-liners

constexpr std::size_t default_grains_per_thread = 4;
// by default we distribute 4 tasks per thread, this number is purely empirical. We don't want to split up
// work into too many tasks (like with 'grain_size = 1'), yet we want it to be a bit more granular than
// doing 1 task per thread since that would be horrible if tasks are noticeably uneven.

// --- Range ---
// -------------

template <class It>
struct Range {
    It          begin;
    It          end;
    std::size_t grain_size;

    Range() = delete;

    constexpr Range(It begin, It end, std::size_t grain_size) : begin(begin), end(end), grain_size(grain_size) {}

    Range(It begin, It end)
        : Range(begin, end, max_size(1, (end - begin) / (hardware_concurrency() * default_grains_per_thread))) {}


    template <class Container, require<has_const_iter<Container>::value> = true>
    Range(const Container& container) : Range(container.begin(), container.end()) {}

    template <class Container, require<has_iter<Container>::value> = true>
    Range(Container& container) : Range(container.begin(), container.end()) {}
}; // requires random-access iterator, but no good way to express that before C++20 concepts

// CTAD for deducing iterator range from a container
template <class Container>
Range(const Container& container) -> Range<typename Container::const_iterator>;

template <class Container>
Range(Container& container) -> Range<typename Container::iterator>;

// --- Index range ---
// -------------------

template <class Idx = std::ptrdiff_t>
struct IndexRange {
    Idx         first;
    Idx         last;
    std::size_t grain_size;

    IndexRange() = delete;

    constexpr IndexRange(Idx first, Idx last, std::size_t grain_size)
        : first(first), last(last), grain_size(grain_size) {}

    IndexRange(Idx first, Idx last)
        : IndexRange(first, last, max_size(1, (last - first) / (hardware_concurrency() * default_grains_per_thread))) {}

    template <class Idx1, class Idx2>
    constexpr IndexRange(Idx1 first, Idx2 last, std::size_t grain_size)
        : first(first), last(last), grain_size(grain_size) {}

    template <class Idx1, class Idx2>
    IndexRange(Idx1 first, Idx2 last)
        : IndexRange(first, last, max_size(1, (last - first) / (hardware_concurrency() * default_grains_per_thread))) {}
};

// Note: It is common to have a ranges from 'int' to 'std::size_t' (for example 'IndexRange{0, vec.size()}'),
//       in such cases we assume 'std::ptrdiff_t' as a reasonable default

// =================
// --- Scheduler ---
// =================

// There is a ton of boilerplate since we need a whole bunch of convenience overloads
// (ranges / index ranges / containers, blocked functions / iteration functions, etc.),
// but conceptually it's all quite simple we end up with 2 overloads for tasks,
// 6+6+3=15 overloads for parallel-for and 4+4+2=10 overloads for reduce

#define utl_parallel_assert_message                                                                                    \
    "Awaitable loops require recursive task support from the 'future_type' of 'Scheduler' backend. "                   \
    "Future can signal such support by  providing a 'using is_recursive = void' member typedef."
// 'static_assert()' only supports string literals, cannot use constexpr variable here, assert itself
// shouldn't be included in the macro as it makes error messages uglier due to macro expansion

template <class Backend = ThreadPool>
struct Scheduler {

    // --- Backend ---
    // ---------------

    Backend backend; // underlying thread pool

    template <class T = void>
    using future_type = typename Backend::template future_type<T>;

    template <class... Args>
    explicit Scheduler(Args&&... args) : backend(std::forward<Args>(args)...) {}

    // --- Task API ---
    // ----------------

    template <class F, class... Args>
    void detached_task(F&& f, Args&&... args) {
        this->backend.detached_task(std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    future_type<R> awaitable_task(F&& f, Args&&... args) {
        return this->backend.awaitable_task(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // --- Parallel-for API ---
    // ------------------------

    // - 'Range' overloads (6) -

    template <class It, class F, require_invocable<F, It, It> = true> // blocked loop iteration overload
    void detached_loop(Range<It> range, F&& f) {
        for (It it = range.begin; it < range.end; it += min_size(range.grain_size, range.end - it))
            this->detached_task(f, it, it + min_size(range.grain_size, range.end - it));
        // 'min_size(...)' bit takes care of the unevenly sized tail segment
    }

    template <class It, class F, require_invocable<F, It> = true> // single loop iteration overload
    void detached_loop(Range<It> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](It low, It high) { // combine individual index
            for (It it = low; it < high; ++it) f(it);                    // calls into blocks and forward
        };                                                               // into a blocked loop iteration overload
        this->detached_loop(range, std::move(iterate_block));
    }

    template <class It, class F, require_invocable<F, It, It> = true>
    void blocking_loop(Range<It> range, F&& f) {
        std::vector<future_type<>> futures;

        for (It it = range.begin; it < range.end; it += min_size(range.grain_size, range.end - it))
            futures.emplace_back(this->awaitable_task(f, it, it + min_size(range.grain_size, range.end - it)));

        for (auto& future : futures) future.wait();
    }

    template <class It, class F, require_invocable<F, It> = true>
    void blocking_loop(Range<It> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](It low, It high) {
            for (It it = low; it < high; ++it) f(it);
        };
        this->blocking_loop(range, std::move(iterate_block));
    }

    template <class It, class F, require_invocable<F, It, It> = true>
    future_type<> awaitable_loop(Range<It> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto submit_loop = [this, range, f = std::forward<F>(f)] { this->blocking_loop(range, f); };
        return this->awaitable_task(std::move(submit_loop));
        // to wait for multiple tasks we need a vector of futures, since logically we want API to return regular
        // 'future_type<>' rather than some composite construct, we can wrap block awaiting into another task
        // that will "merge" all those futures into a single one for the user, this requires recursive task
        // support from the backend so it's provided conditionally
    }

    template <class It, class F, require_invocable<F, It> = true>
    future_type<> awaitable_loop(Range<It> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto iterate_block = [f = std::forward<F>(f)](It low, It high) {
            for (It it = low; it < high; ++it) f(it);
        };
        return this->awaitable_loop(range, std::move(iterate_block));
    }

    // - 'IndexRange' overloads (6) -

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    void detached_loop(IndexRange<Idx> range, F&& f) {
        for (Idx i = range.first; i < range.last; i += static_cast<Idx>(range.grain_size))
            this->detached_task(f, i, static_cast<Idx>(min_size(i + range.grain_size, range.last)));
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    void detached_loop(IndexRange<Idx> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        this->detached_loop(range, std::move(iterate_block));
    }

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    void blocking_loop(IndexRange<Idx> range, F&& f) {
        std::vector<future_type<>> futures;

        for (Idx i = range.first; i < range.last; i += static_cast<Idx>(range.grain_size))
            futures.emplace_back(
                this->awaitable_task(f, i, static_cast<Idx>(min_size(i + range.grain_size, range.last))));

        for (auto& future : futures) future.wait();
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    void blocking_loop(IndexRange<Idx> range, F&& f) {
        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        this->blocking_loop(range, std::move(iterate_block));
    }

    template <class Idx, class F, require_invocable<F, Idx, Idx> = true>
    future_type<> awaitable_loop(IndexRange<Idx> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto submit_loop = [this, range, f = std::forward<F>(f)] { this->blocking_loop(range, f); };
        return this->awaitable_task(std::move(submit_loop));
    }

    template <class Idx, class F, require_invocable<F, Idx> = true>
    future_type<> awaitable_loop(IndexRange<Idx> range, F&& f) {
        static_assert(is_recursive_v<future_type<>>, utl_parallel_assert_message);

        auto iterate_block = [f = std::forward<F>(f)](Idx low, Idx high) {
            for (Idx i = low; i < high; ++i) f(i);
        };
        return this->awaitable_loop(range, std::move(iterate_block));
    }

    // - 'Container' overloads (3) -

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true> // without SFINAE reqs
    void detached_loop(Container&& container, F&& f) {                                         // such overloads would
        this->detached_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));    // always get picked
    }                                                                                          // over the others

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
    void blocking_loop(Container&& container, F&& f) {
        this->blocking_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));
    }

    template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
    future_type<> awaitable_loop(Container&& container, F&& f) {
        return this->awaitable_loop(Range{std::forward<Container>(container)}, std::forward<F>(f));
    }

    // --- Parallel-reduce API ---
    // ---------------------------

    // - 'Range' overloads (2) -

    template <class It, class Op, class R = typename It::value_type>
    R blocking_reduce(Range<It> range, Op&& op) {
        if (range.begin == range.end) throw std::runtime_error("Reduction over an empty range is undefined");

        R          result = *range.begin;
        std::mutex result_mutex;

        this->blocking_loop(Range<It>{range.begin + 1, range.end}, [&](It low, It high) {
            R partial_result = *low;
            for (auto it = low + 1; it != high; ++it) partial_result = op(partial_result, *it);

            const std::scoped_lock result_lock(result_mutex);
            result = op(result, partial_result);
        });

        return result;
    }

    template <class It, class Op, class R = typename It::value_type>
    future_type<R> awaitable_reduce(Range<It> range, Op&& op) {
        auto submit_reduce = [this, range, op = std::forward<Op>(op)] { return this->blocking_reduce(range, op); };
        return this->awaitable_task(std::move(submit_reduce));
    }

    // - 'Container' overloads (2) -

    template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
    R blocking_reduce(Container&& container, Op&& op) {
        return this->blocking_reduce(Range{std::forward<Container>(container)}, std::forward<Op>(op));
    }

    template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
    future_type<R> awaitable_reduce(Container&& container, Op&& op) {
        return this->awaitable_reduce(Range{std::forward<Container>(container)}, std::forward<Op>(op));
    }
};

#undef utl_parallel_assert_message

// =========================
// --- Binary operations ---
// =========================

// Note 1:
// Binary operations should be implemented as functors similarly to 'std::plus<>',
// defining them as free-standing functions poses a huge challenge for inlining due
// to function pointer indirection, this can cause x2-x4 performance degradation.

// Note 2:
// Sum / prod can be defined as aliases for standard functors. Min / max require custom implementation.
// Note the '<void>' specialization for transparent functors, see "transparent function objects" on
// https://en.cppreference.com/w/cpp/functional.html

template <class T = void>
using sum = std::plus<T>;

template <class T = void>
using prod = std::multiplies<T>;

template <class T = void>
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

template <class T = void>
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

// =======================
// --- Global executor ---
// =======================

// A convenient copy of the threadpool & scheduler API hooked up to a global lazily-initialized thread pool

auto& global_scheduler() {
    static Scheduler scheduler;
    return scheduler;
}

// --- Thread pool API ---
// -----------------------

void set_thread_count(std::size_t count = hardware_concurrency()) {
    global_scheduler().backend.set_thread_count(count);
}

std::size_t get_thread_count() { return global_scheduler().backend.get_thread_count(); }

void wait() { global_scheduler().backend.wait(); }

// --- Scheduler API ---
// ---------------------

// Note: We could significantly reduce boilerplate by just using a macro to define functions forwarding everything to
//       global scheduler and returning auto, however this messes up LSP autocomplete for users, so we do it manually

// - Task API -

template <class F, class... Args>
void detached_task(F&& f, Args&&... args) {
    global_scheduler().detached_task(std::forward<F>(f), std::forward<Args>(args)...);
}

template <class F, class... Args, class R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
Future<R> awaitable_task(F&& f, Args&&... args) {
    return global_scheduler().awaitable_task(std::forward<F>(f), std::forward<Args>(args)...);
}

// - Parallel-for API -

template <class It, class F>
void detached_loop(Range<It> range, F&& f) {
    global_scheduler().detached_loop(range, std::forward<F>(f));
}

template <class It, class F>
void blocking_loop(Range<It> range, F&& f) {
    global_scheduler().blocking_loop(range, std::forward<F>(f));
}

template <class It, class F>
auto awaitable_loop(Range<It> range, F&& f) {
    return global_scheduler().awaitable_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
void detached_loop(IndexRange<Idx> range, F&& f) {
    global_scheduler().detached_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
void blocking_loop(IndexRange<Idx> range, F&& f) {
    global_scheduler().blocking_loop(range, std::forward<F>(f));
}

template <class Idx, class F>
Future<> awaitable_loop(IndexRange<Idx> range, F&& f) {
    return global_scheduler().awaitable_loop(range, std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
void detached_loop(Container&& container, F&& f) {
    global_scheduler().detached_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
void blocking_loop(Container&& container, F&& f) {
    global_scheduler().blocking_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class Container, class F, require_has_some_iter<std::decay_t<Container>> = true>
Future<> awaitable_loop(Container&& container, F&& f) {
    return global_scheduler().awaitable_loop(std::forward<Container>(container), std::forward<F>(f));
}

template <class It, class Op, class R = typename It::value_type>
R blocking_reduce(Range<It> range, Op&& op) {
    return global_scheduler().blocking_reduce(range, std::forward<Op>(op));
}

template <class It, class Op, class R = typename It::value_type>
Future<R> awaitable_reduce(Range<It> range, Op&& op) {
    return global_scheduler().awaitable_reduce(range, std::forward<Op>(op));
}

template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
R blocking_reduce(Container&& container, Op&& op) {
    return global_scheduler().blocking_reduce(std::forward<Container>(container), std::forward<Op>(op));
}

template <class Container, class Op, class R = typename std::decay_t<Container>::value_type>
Future<R> awaitable_reduce(Container&& container, Op&& op) {
    return global_scheduler().awaitable_reduce(std::forward<Container>(container), std::forward<Op>(op));
}

} // namespace utl::parallel::impl

// ______________________ PUBLIC API ______________________

namespace utl::parallel {

using impl::Scheduler;
using impl::ThreadPool;
using impl::Future;

using impl::Range;
using impl::IndexRange;

using impl::sum;
using impl::prod;
using impl::min;
using impl::max;

using impl::set_thread_count;
using impl::get_thread_count;
using impl::wait;

using impl::detached_task;
using impl::awaitable_task;

using impl::detached_loop;
using impl::blocking_loop;
using impl::awaitable_loop;

using impl::blocking_reduce;
using impl::awaitable_reduce;

namespace this_thread = impl::this_thread;

using impl::hardware_concurrency;

} // namespace utl::parallel

#endif
#endif // module utl::parallel
