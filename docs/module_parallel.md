[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::parallel

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/parallel.hpp)

**utl::parallel** module is a lightweight threading library providing API somewhat similar to [Intel TBB](https://github.com/uxlfoundation/oneTBB).

It implements classic building blocks of concurrent algorithms such as tasks, parallel for, reductions and etc. and provides a sane thread pool implementation with work-stealing and recursive parallelism support, in summary:

- Tasks, parallel loops and reductions
- Work-stealing thread pool
- Support for recursive parallelism
- Focus on simple & uniform API

> [!Tip]
> [Benchmarks](#benchmarks) and [usage examples](#examples) can be found at the bottom.

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Definitions

```cpp
// Scheduler
template <class Backend = ThreadPool>
struct Scheduler {
    Backend backend; // underlying thread pool
    
    template <class T = void> using future_type = typename Backend::future_type<T>;
    
    // Task API
    template <class F, class... Args>           void  detached_task(F&& f, Args&&... args);
    template <class F, class... Args> future_type<R> awaitable_task(F&& f, Args&&... args);
    
    // Parallel-for API
    template <class It, class F>          void  detached_loop(Range<It> range, F&& f);
    template <class It, class F>          void  blocking_loop(Range<It> range, F&& f);
    template <class It, class F> future_type<> awaitable_loop(Range<It> range, F&& f);
    
    template <class Idx, class F>          void  detached_loop(IndexRange<Idx> range, F&& f);
    template <class Idx, class F>          void  blocking_loop(IndexRange<Idx> range, F&& f);
    template <class Idx, class F> future_type<> awaitable_loop(IndexRange<Idx> range, F&& f);
    
    template <class Container, class F>          void  detached_loop(Container&& container, F&& f);
    template <class Container, class F>          void  blocking_loop(Container&& container, F&& f);
    template <class Container, class F> future_type<> awaitable_loop(Container&& container, F&& f);
    
    // Parallel-reduce API
    template <class It, class Op>             R   blocking_reduce(Range<It> range, Op&& op);
    template <class It, class Op> future_type<R> awaitable_reduce(Range<It> range, Op&& op);
    
    template <class Container, class Op>             R   blocking_reduce(Container&& container, Op&& op);
    template <class Container, class Op> future_type<R> awaitable_reduce(Container&& container, Op&& op);
};

// Thread pool
struct ThreadPool {
    // Initialization
    explicit ThreadPool(std::size_t count = hardware_concurrency());
     
    // Threading
    void        set_thread_count(std::size_t count = hardware_concurrency());
    std::size_t get_thread_count() const noexcept;
    
    // Task queuing
    template <class F, class... Args>           void  detached_task(F&& f, Args&&... args);
    template <class F, class... Args> future_type<R> awaitable_task(F&& f, Args&&... args);
    
    void wait();
    
    // Future
    template <class T = void> future_type { /* Same API as std::future<T> */ };
};

template <class T = void>
using Future = ThreadPool::future_type<T>;

// Ranges
template <class It>
struct Range {
    Range() = delete;
    Range(It begin, It end);
    Range(It begin, It end, std::size_t grain_size);
    
    template <class Container> Range(      Container& container);
    template <class Container> Range(const Container& container);
};

template <class Idx = std::ptrdiff_t>
struct IndexRange {
    IndexRange() = delete;
    IndexRange(Idx first, Idx last);
    IndexRange(Idx first, Idx last, std::size_t grain_size);
}

// Binary operations
template <class T = void> struct  sum { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T = void> struct prod { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T = void> struct  min { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T = void> struct  max { constexpr T operator()(const T& lhs, const T& rhs) const; }

// Global scheduler
/* 'Scheduler'  API, but placed directly into the namespace */
/* 'ThreadPool' API, but placed directly into the namespace */
/* This will use global lazily-initialized thread pool      */

// Thread introspection
namespace this_thread {
    std::optional<      void*> get_pool () noexcept;
    std::optional<std::size_t> get_index() noexcept;
}

std::size_t hardware_concurrency() noexcept;
```

> [!Important]
> There is no tight coupling between the `Scheduler<>` and the `ThreadPool`, other implementation may be used assuming they provide the 2 methods for submitting tasks.

## Methods

### Scheduler

#### Task API

> ```cpp
> template <class F, class... Args> void detached_task(F&& f, Args&&... args);
> ```

Launches asynchronous task to execute callable `f` with arguments `args...`.

**Note:** [Callables](https://en.cppreference.com/w/cpp/named_req/Callable) constitute any objects with a defined `operator()`, for example: function pointers, functors, lambdas, [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function), [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task), [`std::bind()`](https://en.cppreference.com/w/cpp/utility/functional/bind.html) result and etc.

> ```cpp
> template <class F, class... Args> future_type<R> awaitable_task(F&& f, Args&&... args);
> ```

Launches asynchronous task to execute callable `f` with arguments `args...` and returns its [`std::future`](https://en.cppreference.com/w/cpp/thread/future).

#### Parallel-for API

> ```cpp
> template <class It, class F>          void  detached_loop(Range<It> range, F&& f);
> template <class It, class F>          void  blocking_loop(Range<It> range, F&& f);
> template <class It, class F> future_type<> awaitable_loop(Range<It> range, F&& f);
> ```

Detached / blocking / awaitable parallel-for loop over an **iterator range**.

Loop body `f` can be defined in two ways:

**1.** Single-argument `f(it)`, defining the body of a single loop iteration.

**2.** Two-argument `f(low, high)`, defining how to execute a part of the loop spanning from `low` to `high`.

**Note 1:** For most use cases **(1)** is a more intuitive option, closely resembling how we write a regular serial loop.

**Note 2:** `It` is assumed to be a [random access iterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator).

> ```cpp
> template <class Idx, class F>          void  detached_loop(IndexRange<Idx> range, F&& f);
> template <class Idx, class F>          void  blocking_loop(IndexRange<Idx> range, F&& f);
> template <class Idx, class F> future_type<> awaitable_loop(IndexRange<Idx> range, F&& f);
> ```

Detached / blocking / awaitable parallel-for loop over an **index range**.

Like in the iterator case, loop body `f` can be defined both for a single iteration and as block.

> ```cpp
> template <class Container, class F>          void  detached_loop(Container&& container, F&& f);
> template <class Container, class F>          void  blocking_loop(Container&& container, F&& f);
> template <class Container, class F> future_type<> awaitable_loop(Container&& container, F&& f);
> ```

Detached / blocking / awaitable parallel-for loop over an **iterator range** spanning `container.begin()` to `container.end()`.

Like in the iterator case, loop body `f` can be defined both for a single iteration and as block.

#### Parallel-reduce API

> ```cpp
> template <class It, class Op>             R   blocking_reduce(Range<It> range, Op&& op);
> template <class It, class Op> future_type<R> awaitable_reduce(Range<It> range, Op&& op);
> ```

Detached / blocking / awaitable parallel reduction over a binary operator `op` over an **iterator range**.

Binary operator `op` is defined by the signature `T(const T&, const T&)` where `T` corresponds to iterator `::value_type`.

**Note:** Most commonly used to compute vector sum / product / min / max in parallel, see [pre-defined binary operation](#binary-operations).

> ```cpp
> template <class Container, class Op>             R   blocking_reduce(Container&& container, Op&& op);
> template <class Container, class Op> future_type<R> awaitable_reduce(Container&& container, Op&& op);
> ```

Detached / blocking / awaitable parallel reduction over a binary operator `op` over an **iterator range** spanning `container.begin()` to `container.end()`.

### Thread pool

#### Initialization

> ```cpp
> explicit ThreadPool(std::size_t count = hardware_concurrency());
> ```

Creates thread pool with `count` threads.

#### Threading

> ```cpp
> void set_thread_count(std::size_t count = hardware_concurrency());
> ```

Changes the number of threads in the thread pool.

**Note:** This method will block & wait for all in-flight tasks to be completed before resizing the thread pool.

> ```cpp
> std::size_t get_thread_count() const noexcept;
> ```

Returns number of threads in the thread pool.

#### Task queuing

> ```cpp
> template <class F, class... Args> void detached_task(F&& f, Args&&... args);
> ```

Launches asynchronous task to execute callable `f` with arguments `args...`.

> ```cpp
> template <class F, class... Args> future_type<R> awaitable_task(F&& f, Args&&... args);
> ```

Launches asynchronous task to execute callable `f` with arguments `args...` and returns its [`std::future`](https://en.cppreference.com/w/cpp/thread/future).

> ```cpp
> void wait();
> ```

Blocks current thread until all in-flight tasks are completed.

#### Future

> ```cpp
> template <class T = void> future_type { /* Same API as std::future<T> */ };
> ```

A thin wrapper around [`std::future`](https://en.cppreference.com/w/cpp/thread/future) used by the thread pool. Exposes the same API as a standard future and can be constructed from it.

This future allows the usage of recursive await-able tasks. When waited with `get()` / `wait()` / `wait_for()` / `wait_until()` instead of just blocking it first looks for recursive work to do in the meantime.

> ```cpp
> template <class T = void>
> using Future = ThreadPool::future_type<T>;
> ```

Alias for `ThreadPool::future_type<T>` placed at the namespace level.

### Ranges

> ```cpp
> template <class It>
> struct Range {
>     Range() = delete;
>     Range(It begin, It end);
>     Range(It begin, It end, std::size_t grain_size);
> 
>     template <class Container> Range(      Container& container);
>     template <class Container> Range(const Container& container);
> };
> ```

A lightweight struct representing an **iterator range**.

Constructor **(2)** creates a range spanning begin to end and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of grain_size.

Constructors **(4)** and **(5)** create a range spanning `container.begin()` to `container.end()` for any container that supports standard member types `Container::iterator` and `Container::const_iterator`. Grain size is selected automatically.

**Note:** `grain_size` is a maximum size of subranges, in which the main range gets split up for parallel execution. Splitting up workload into smaller grains can be beneficial for tasks with unpredictable or uneven complexity, but increases the overhead of scheduling & synchronization. By default, the workload is split into `parallel::hardware_concurrency() * 4` grains.

> ```cpp
> template <class Idx = std::ptrdiff_t>
> struct IndexRange {
>     IndexRange() = delete;
>     IndexRange(Idx first, Idx last);
>     IndexRange(Idx first, Idx last, std::size_t grain_size);
> }
> ```

A lightweight struct representing an **index range**.

Constructor **(2)** creates a range spanning `first` to `last` and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of `grain_size`.

**Note:** Like all standard ranges, index range is **exclusive** and does not include `last`.

### Binary operations

```cpp
template <class T> struct  sum { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct prod { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  min { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  max { constexpr T operator()(const T& lhs, const T& rhs) const; }

// Transparent specializations
template<> struct  sum<void> { template<class T, class U> constexpr auto operator()(T&& lhs, T&& rhs) const; }
template<> struct prod<void> { template<class T, class U> constexpr auto operator()(T&& lhs, T&& rhs) const; }
template<> struct  min<void> { template<class T, class U> constexpr auto operator()(T&& lhs, T&& rhs) const; }
template<> struct  max<void> { template<class T, class U> constexpr auto operator()(T&& lhs, T&& rhs) const; }
```

Pre-defined binary operations for `parallel::reduce()`.

**Note 1:** All functors will be `noexcept` if possible.

**Note 2:** "Transparent functors" are `void` specializations that deduce their parameter and return types from the arguments. This is how function objects should usually be used. See [cppreference](https://en.cppreference.com/w/cpp/utility/functional#Transparent_function_objects) for details.

### Global scheduler

For user convenience all `Sheduler<>` and `ThreadPool` methods are also doubled at the namespace scope, in which case they use a global lazily-initialized `Scheduler<>` with a `ThreadPool` backend. See [examples](#examples).

### Thread introspection

> ```cpp
> namespace this_thread {
>     std::optional<      void*> get_pool () noexcept;
>     std::optional<std::size_t> get_index() noexcept;
> }
> ```

Returns thread index or type-erased pointer to the parent thread pool for the current thread.

Returns [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) if current thread does not belong to a thread pool.

**Note 1:** Similar to functions from [`std::this_thread`](https://en.cppreference.com/w/cpp/header/thread.html).

**Note 2:** Can be supported by any custom thread pool, assuming it sets the appropriate variables.

## Examples

### Detached tasks

[ [Run this code]() ]

```cpp
using namespace utl;

const std::string message = "<some hypothetically very large message>";

// Log the message asynchronously
parallel::detached_task([&]{ std::ofstream("log.txt") << message; });

// ... do some work in the meantime ...

parallel::wait(); // wait for tasks to complete
```

### Awaitable tasks

[ [Run this code]() ]

```cpp
double some_heavy_computation(double x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x + 32;
}

// ...

using namespace utl;

// Launch asynchronous computation
auto future = parallel::awaitable_task(some_heavy_computation, 10);

// ... do some work in the meantime ...

// Await the result
const double result = future.get();

assert( result == 42 );
```

### Parallel for loop

[ [Run this code]() ]

```cpp
double f(double x) { return std::exp(std::sin(x)); }

// ...

using namespace utl;

std::vector<double> vals(200'000, 0.5);

// (optional) Select the number of threads 
parallel::set_thread_count(8);

// Apply f() to all elements of the vector
parallel::blocking_loop(vals, [&](auto it) { *it = f(*it); });

// Apply f() to indices [0, 100)
parallel::blocking_loop(parallel::IndexRange{0, 100}, [&](int i) { vals[i] = f(vals[i]); });

// Specify computation in blocks instead of element-wise
parallel::blocking_loop(parallel::IndexRange{0, 100}, [&](int low, int high) {
    for (int i = low; i < high; ++i) vals[i] = f(vals[i]);
});
```

### Reducing over a binary operation

[ [Run this code]() ]

```cpp
using namespace utl;

const std::vector<double> vals(200'000, 2);

// Reduce container over a binary operation
const double sum = parallel::blocking_reduce(vals, parallel::sum<>());

assert( sum == 200'000 * 2 );

// Reduce iterator range over a binary operation
const double subrange_sum = parallel::blocking_reduce(parallel::Range{vals.begin() + 100, vals.end()}, parallel::sum<>{});

assert( subrange_sum == (200'000 - 100) * 2 );
```

### Using a local thread pool

[ [Run this code]() ]

```cpp
using namespace utl;

parallel::ThreadPool pool; // uses as many threads as it detects

pool.detached_task([]{ std::cout << "Hello from the task\n"; });

pool.set_thread_count(0); // will wait for the tasks, join all threads and release the memory
```

Output:

```
```

### Recursive tasks

[ [Run this code]() ]

```cpp
using namespace utl;

// Deeply recursive illustrative task, not a practical way of computing fibonacci numbers
std::function<int(int)> fibonacci = [&](int n) {
    if (n < 2) return n;

    auto future_prev_1 = parallel::awaitable_task([&] { return fibonacci(n - 1); });
    auto future_prev_2 = parallel::awaitable_task([&] { return fibonacci(n - 2); });

    return future_prev_1.get() + future_prev_2.get();
};

assert( fibonacci(8) == 21 );
```

### Awaitable parallel loop with specific grain size

[ [Run this code]() ]

```cpp
using namespace utl;

std::vector<int> a(200'000, 27);
std::vector<int> b(200'000, 13);
std::vector<int> c(200'000,  0);

// Launch the task to compute { c = a + b } in parallel, we know this
// workload is very even so we can use coarser grains that by default
const std::size_t grain_size = 200'000 / parallel::get_thread_count();

auto future = parallel::awaitable_loop(parallel::IndexRange<std::size_t>{0, 200'000, grain_size},
    [&](std::size_t i){ c[i] = a[i] + b[i]; }
);

// ... do some work in the meantime ...

// Await the result
future.wait();

for (std::size_t i = 0; i < 200'000; ++i) assert( c[i] == a[i] + b[i] );
```

### Thread introspection

[ [Run this code]() ]

```cpp
using namespace utl;

// In debugging it is often useful to have some information about the thread whereabouts,
// we can detect if current thread belongs to a pool, which pool and at which index
auto future = parallel::awaitable_task([]{
    std::cout << "I am a thread [" << *parallel::this_thread::get_index() << "]"
              << " in the pool  [" << *parallel::this_thread::get_pool()  << "]" 
              << std::endl;
});
future.wait();

auto std_future = std::async([]{
    std::cout << "Am I a pool thread? -> " << (parallel::this_thread::get_pool() ? "true" : "false")
              << std::endl;
});
std_future.wait();
```

Output:

```
```

## Motivation

### The problem

Naive use of [std::async](https://en.cppreference.com/w/cpp/thread/async) and [std::thread](https://en.cppreference.com/w/cpp/thread/thread) suffers greatly from the overhead of thread creation, this overhead makes small tasks (under a few ms) largely inefficient and introduces a great degree of performance instability. 

[Thread pools](https://en.wikipedia.org/wiki/Thread_pool) are a classic solution to this problem. There are many existing implementations in C++, most of them tend to use a shared-queue design. This approach is easy to implement and works for simple parallelization, however it doesn't scale well as tasks get more granular.

There is also a problem of recursive workloads, something as simple as splitting a task into awaitable subtasks will deadlock the **vast majority** of thread pool implementations found online (even the ones that do implement work-stealing as they tend to work only with detached recursion), which was the initial motivation behind writing this library.

### Why work-stealing

Large concurrency frameworks such as [OpenMP](https://en.wikipedia.org/wiki/OpenMP) and [Intel TBB](https://github.com/uxlfoundation/oneTBB) tend to use work-stealing design where each thread keeps its own queue of tasks and steals work from the backs of other queues when its own queue gets exhausted. Such approach introduces a whole slew of potential benefits:

1. We can offload task scheduling to idle threads
2. We can reduce lock contention by splitting scheduling over multiple queues
3. Pushing/executing local tasks in the Last-In-First-Out (LIFO) order is likely to keep work hot in cache  
4. Stealing in the First-In-First-Out (FIFO) order is likely to pull larger tasks (as they will be closer to the recursion root), minimizing contention over small task scheduling

Recursive workloads in general are also quite difficult to sensibly implement with a single queue which is why work-stealing approach is usually used.

All of this makes work-stealing a rather good general case solution as it tends to perform well on most workloads.

## Benchmarks

 `utl::parallel` does not claim to be superior to vendor-optimized concurrency frameworks, what it does is expose a simple threading API with a concise (~800 L.O.C.) implementation written entirely in standard C++17. As of now this niche seems to be rather empty as there is almost no stand-alone thread pools supporting recursion.

Below are a few [benchmarks](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks/module_parallel/) comparing performance of `utl::parallel` with other thread pools, frameworks and standard approaches on various tasks:

#### Recursive task graph

#### Uneven tasks with some recursion

#### Uneven non-recursive tasks

| Approach                     | Speedup | Notes                                   |
| ---------------------------- | ------- | --------------------------------------- |
| Serial                       | `100%`  | Reference                               |
| `std::thread`                | `xxx%`  | Unstable results due to thread creation |
| `std::async()`               | `xxx%`  | Unstable results due to thread creation |
| `parallel::awaitable_task()` | `xxx%`  |                                         |
| `BS::thread_pool`            | `xxx%`  |                                         |
| `progschj/ThreadPool`        | `xxx%`  |                                         |

#### Even non-recursive tasks

### Questions & answers

**Q:** Why use custom future type instead of `std::future<>`?

**A:** This is necessary for recursive tasks. When `wait()`'ed from a pool thread instead of blocking right-away it first checks if there any recursive subtasks tasks to do in the meantime. The usual future would deadlock in this case by design, no way around it. Other than that this is a very thin wrapper around a standard `std::future<>`.

**Q:** Are there any improvement to be made in terms of performance?

**A:** "Ideal" work-stealing executor would likely use a global lock-free MPMC queue for external tasks and per-thread Chase-Lev SPMC lock-free queues for task stealing. "Ideal" thread pool would also use a custom implementation of `std::function<>` (something closer to `std::move_only_function<>` from C++23). Unfortunately all of those things are highly complex and there are very few clean and correct implementations out there. A proper lock-free MPMC queue implementation with exception correctness would be higher in size and complexity than this entire library, the same applies to custom delegates and SPMC queues, which is why they are usually pulled in as dependencies. This implementation tries to do the best it can while keeping the thread pool logic simple enough to be copy-pastable into a different project.

**Q:** Is it possible to cause deadlocks?

**A:** Only by manually introducing an inter-thread dependency on a shared resource, in which case the deadlock is logical and no thread pool design could possibly prevent this.

**Q:** How was this library tested?

**A:** It has a wide [testing suite]() covering multiple algorithms and use cases that runs with Thread Sanitizer (GCC / clang), Address Sanitizer (GCC / clang / MSVC) and Undefined Behavior Sanitizer (GCC / clang) on all major platforms. Tests also include some reasonable fuzzing.

**Q:** Why no benchmarks against [Leopard](https://github.com/hosseinmoein/Leopard), [riften::ThiefPool](https://github.com/ConorWilliams/Threadpool) and [dp::thread_pool](https://github.com/DeveloperPaul123/thread-pool)?

**A:** All of these require **C++20** to build, which is above the required standard for building this repo. `riften::ThiefPool` also includes several dependencies. Out of these 3 implementation, recursive workload seems to only be supported by `Leopard` and even that requires some algorithm adjustments on user-side with `run_task()` needing to be called in the right places to avoid a deadlock.