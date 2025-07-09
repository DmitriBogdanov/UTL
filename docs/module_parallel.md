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

**utl::parallel** module is a lightweight threading library providing an API very similar to [Intel TBB](https://github.com/uxlfoundation/oneTBB).

It implements classic building blocks of concurrent algorithms such as tasks, parallel for, reductions and etc. and provides a sane thread pool implementation for custom concurrency needs.

> [!Important]
> Due to rather extensive API, seeing [usage examples](#examples) first might be helpful.

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Definitions

```cpp
// Thread pool
class ThreadPool {
    // Construction
    ThreadPool() = default;
    explicit ThreadPool(std::size_t thread_count);
    ~ThreadPool();
    
    // Threads
    std::size_t get_thread_count() const;
    void        set_thread_count(std::size_t thread_count);
    
    // Task queue
    template <class Func, class... Args>
    void add_task(Func&& func, Args&&... args);
    
    template <class Func, class... Args>
    std::future<FuncReturnType> add_task_with_future(Func&& func, Args&&... args);
    
    void wait_for_tasks();
    void clear_task_queue();
    
    // Pausing
    void     pause();
    void   unpause();
    bool is_paused() const;
};

// Static thread pool operations
ThreadPool& static_thread_pool();

std::size_t get_thread_count();
void        set_thread_count(std::size_t thread_count);

// Ranges
template <class Iter>
struct Range {
    Range() = delete;
    Range(Iter begin, Iter end);
    Range(Iter begin, Iter end, std::size_t grain_size);
    
    template <class Container> Range(const Container& container);
    template <class Container> Range(      Container& container);
}

template <class Idx>
struct IndexRange {
    IndexRange() = delete;
    IndexRange(Idx first, Idx last);
    IndexRange(Idx first, Idx last, std::size_t grain_size);
}

// Task API
template <class Func, class... Args> void task(Func&& func, Args&&... args);

template <class Func, class... Args>
std::future<FuncReturnType>   task_with_future(Func&& func, Args&&... args);

void wait_for_tasks();

// Parallel-for API
template <class Iter,      class Func> void for_loop(     Range<Iter> range,     Func&& func);
template <class Container, class Func> void for_loop(const Container& container, Func&& func);
template <class Container, class Func> void for_loop(      Container& container, Func&& func);
template <class Idx,       class Func> void for_loop( IndexRange<Idx> range,     Func&& func);

// Reduction API
template <std::size_t unroll = 1, class Iter,      class BinaryOp>
auto reduce(     Range<Iter> range,     BinaryOp&& op) -> typename Iter::value_type;

template <std::size_t unroll = 1, class Container, class BinaryOp>
auto reduce(const Container& container, BinaryOp&& op) -> typename Container::value_type;

template <std::size_t unroll = 1, class Container, class BinaryOp>
auto reduce(      Container& container, BinaryOp&& op) -> typename Container::value_type;

// Pre-defined binary operations
template <class T> struct  sum { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct prod { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  min { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  max { constexpr T operator()(const T& lhs, const T& rhs) const; }
```

> [!Important]
> In most application there is no need to ever work with `ThreadPool` directly, all of the work will be automatically done by `parallel::get_thread_count()`, `parallel::set_thread_count()`, `parallel::task()`, `parallel::for_loop()`, `parallel::reduce()` and etc.

## Methods

### Thread pool

#### Construction

```cpp
ThreadPool() = default;
```

```cpp
explicit ThreadPool(std::size_t thread_count);
```

Creates thread pool with `thread_count` worker threads.

```cpp
~ThreadPool();
```

Finishes all tasks left in the queue then shuts down worker threads.

If the pool was paused, it will automatically resume to finish the work.

#### Threads

```cpp
std::size_t ThreadPool::get_thread_count();
```

Returns current number of worker threads in the thread pool.

```cpp
void ThreadPool::set_thread_count(std::size_t thread_count);
```

Changes the number of worker threads managed by the thread pool to `thread_count`.

#### Task queue

```cpp
template <class Func, class... Args>
void add_task(Func&& func, Args&&... args);
```

Adds a task to execute callable `func` with arguments `args...` (`args...` may be empty).

**Note:** Callables include: function pointers, functors, lambdas, [std::function](https://en.cppreference.com/w/cpp/utility/functional/function), [std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task) and etc.

```cpp
template <class Func, class... Args>
std::future<FuncReturnType> add_task_with_future(Func&& func, Args&&... args);
```

Adds a task to execute callable `func` with arguments `args...` (`args...` may be empty) and returns its [std::future](https://en.cppreference.com/w/cpp/thread/future).

**Note:** `FuncReturnType` evaluates to the return type of the callable `func`.

```cpp
void wait_for_tasks();
```

Blocks current thread execution until all queued tasks are finished.

```cpp
void ThreadPool::clear_task_queue();
```

Clears all currently queued tasks. Tasks already in progress continue running until finished.

#### Pausing

```cpp
void ThreadPool::pause();
```

Stops execution of new tasks from the queue. Use `.unpause()` to resume. Tasks already in progress continue running until finished.

```cpp
void ThreadPool::unpause();
```

Resumes execution of queued tasks.

```cpp
bool ThreadPool::is_paused() const;
```

Returns whether the thread pool is paused.

### Static thread pool

```cpp
ThreadPool& static_thread_pool();
```

Returns a global static instance of the thread pool.

In most cases there is no need to manually maintain the thread pool at call-site, a global thread pool instance gets created automatically upon the first call to `parallel::static_thread_pool()` or any of the parallel algorithm functions.

**Note:** In most cases even the global instance doesn't have to be directly accessed thorough this method, all threading logic will be automatically managed by `parallel::get_thread_count()`, `parallel::set_thread_count()`, `parallel::task()`, `parallel::for_loop()`, `parallel::reduce()` and etc.

```cpp
std::size_t get_thread_count();
```

Returns current number of worker threads in the static thread pool.

```cpp
void set_thread_count(std::size_t thread_count);
```

Changes the number of worker threads managed by the static thread pool to `thread_count`.

### Ranges

```cpp
template <class Iter>
struct Range {
    Range() = delete;
    Range(Iter begin, Iter end);
    Range(Iter begin, Iter end, std::size_t grain_size);
    
    template <class Container> Range(const Container& container);
    template <class Container> Range(      Container& container);
}
```

A lightweight wrapper representing an **iterator range**.

Constructor **(2)** creates a range spanning `begin` to `end` and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of `grain_size`.

Constructors **(4)** and **(5)** create a range spanning `container.begin()` to `container.end()` for any container that supports standard member types `Container::iterator` and `Container::const_iterator`.

**Note:** In this context, `grain_size` is a maximum size of subranges, in which the main range gets split up for parallel execution. Splitting up workload into smaller grains can be beneficial for tasks with unpredictable or uneven complexity, but increases the overhead of scheduling & synchronization. By default, the workload is split into `parallel::get_thread_count() * 4` grains.

```cpp
template <class Idx>
struct IndexRange {
    IndexRange() = delete;
    IndexRange(Idx first, Idx last);
    IndexRange(Idx first, Idx last, std::size_t grain_size);
}
```

A lightweight wrapper representing an **index range**.

Constructor **(2)** creates a range spanning `first` to `last` and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of `grain_size`.

**Note:** Like all the standard ranges, index range is **exclusive** and does not include `last`.

### Task API

```cpp
template <class Func, class... Args> void task(Func&& func, Args&&... args);
```

Launches asynchronous task to execute callable `func` with arguments `args...`.

```cpp
template <class Func, class... Args>
std::future<FuncReturnType>   task_with_future(Func&& func, Args&&... args);
```

Launches asynchronous task to execute callable `func` with arguments `args...` and returns its [std::future](https://en.cppreference.com/w/cpp/thread/future).

```cpp
void wait_for_tasks();
```

Waits for all currently launched tasks to finish.

### Parallel-for API

```cpp
template <class Iter,      class Func> void for_loop(     Range<Iter> range,     Func&& func);
template <class Container, class Func> void for_loop(const Container& container, Func&& func);
template <class Container, class Func> void for_loop(      Container& container, Func&& func);
```

Executes parallel `for` loop over a range `range` where `func` is a callable with a signature `void(Iter low, Iter high)` that defines how to compute a part of the `for` loop. See the [examples](#parallel-for-loop).

Overloads **(2)** and **(3)** construct range spanning `container.begin()` to `container.end()` automatically.

```cpp
template <class Idx,       class Func> void for_loop( IndexRange<Idx> range,     Func&& func);
```

Executes parallel `for` loop over an **index range** `range` where `func` is a callable with a signature `void(Idx low, Idx high)` that defines how to compute a part of the `for` loop.

### Reduction API

```cpp
template <std::size_t unroll = 1, class Iter,      class BinaryOp>
auto reduce(     Range<Iter> range,     BinaryOp&& op) -> typename Iter::value_type;

template <std::size_t unroll = 1, class Container, class BinaryOp>
auto reduce(const Container& container, BinaryOp&& op) -> typename Container::value_type;

template <std::size_t unroll = 1, class Container, class BinaryOp>
auto reduce(      Container& container, BinaryOp&& op) -> typename Container::value_type;
```

Reduces range `range`  over the binary operation `op` in parallel.

Overloads **(2)** and **(3)** construct range spanning `container.begin()` to `container.end()` automatically.

`unroll` template parameter can be set to automatically unroll reduction loops with a given step, which oftentimes aids compiler with vectorization. By default, no loop unrolling takes place.

**Note 1:** Binary operation can be anything with a signature `T(const T&, const T&)` or `T(T, T)`.

**Note 2:** Be wary of passing binary operations as function pointers since that makes inlining more difficult. Lambdas and functor-classes don't experience the same issue, see [pre-defined binary operations](#pre-defined-binary-operations).

**Note 3:** It is not unusual to see super-linear speedup with `unroll` set to `4`, `8`, `16` or `32`. Reduction loops are often difficult to vectorize otherwise due to reordering of float operations. Performance impact is hardware- and architecture- dependent.

#### Pre-defined binary operations

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

## Examples

### Launching async tasks

[ [Run this code](https://godbolt.org/z/Y3fan7Tzo) ]

```cpp
using namespace utl;

const std::string message = "<some hypothetically very large message>";

// Log the message asynchronously
parallel::task([&]{ std::ofstream("log.txt") << message; });

// ... do some other work in the meantime ...

// Destructor will automatically wait for ongoing tasks to finish
// before exiting 'main()', we can also trigger the wait manually
parallel::wait_for_tasks();
```

### Launching async tasks with future

[ [Run this code](https://godbolt.org/z/Gc1ejha3x) ]

```cpp
double some_heavy_computation(double x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x + 32;
}

// ...

using namespace utl;

// Launch the computation asynchronously and get its future
auto future = parallel::task_with_future(some_heavy_computation, 10);

// ... do some other work in the meantime ...

// Get the value from std::future, this will wait until the task is finished
const double result = future.get();

assert( result == 42 );
```

### Parallel for loop

[ [Run this code](https://godbolt.org/z/xq9G9138q) ]

```cpp
double f(double x) { return std::exp(std::sin(x)); }

// ...

using namespace utl;

std::vector<double> vals(400'000, 0.5);

// (optional) Select the number of threads 
parallel::set_thread_count(8);

// Apply f() to all elements of the vector
parallel::for_loop(vals, [&](auto low, auto high) {
    for (auto it = low; it != high; ++it) *it = f(*it);
});

// Apply f() to the fist half of the vector
parallel::for_loop(parallel::IndexRange<std::size_t>{0, vals.size() / 2}, [&](auto low, auto high) {
    for (auto i = low; i != high; ++i) vals[i] = f(vals[i]);
});
```

### Reducing over a binary operation

[ [Run this code](https://godbolt.org/z/54KToWo8E) ]

```cpp
using namespace utl;

const std::vector<double> vals(200'000, 2);

// Reduce container over a binary operation
const double sum = parallel::reduce(vals, parallel::sum<double>());

assert( sum == 200'000 * 2 );

// Reduce range over a binary operation
const double subrange_sum = parallel::reduce(parallel::Range{vals.begin() + 100, vals.end()}, parallel::sum<>());

assert( subrange_sum == (200'000 - 100) * 2 );
```

## Benchmarks

While `utl::parallel` does not claim to provide superior performance to complex vendor-optimized libraries such as [OpenMP](https://en.wikipedia.org/wiki/OpenMP), [Intel TBB](https://github.com/uxlfoundation/oneTBB), [MPI](https://www.open-mpi.org) and etc., it provides a significant boost in both speed and convenience relative to the explicit use of [std::async](https://en.cppreference.com/w/cpp/thread/async) and [std::thread](https://en.cppreference.com/w/cpp/thread/thread) due to its ability to reuse threads and automatically distribute workload.

Below are some of the [benchmarks](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks/module_parallel/) comparing performance of different approaches on trivially parallelizable tasks:

```
====== BENCHMARKING ON: Parallel vector sum ======

Threads           -> 4
N                 -> 25000000
Data memory usage -> 190.73486328125 MiB

| relative |               ms/op |                op/s |    err% |     total | Parallel vector sum
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------
|   100.0% |               18.86 |               53.03 |    2.1% |      2.34 | `Serial version`
|   380.0% |                4.96 |              201.53 |    0.1% |      0.61 | `OpenMP reduce`
|   288.2% |                6.54 |              152.83 |    1.1% |      0.88 | `Naive std::async()`
|   373.5% |                9.99 |              100.13 |    0.2% |      1.22 | `parallel::reduce()`
|   430.9% |                8.66 |              115.41 |    0.7% |      1.06 | `parallel::reduce<4>() (loop unrolling enabled)`

|--------------------------------------------------|--------------------|
|                                            Method|         Control sum|
|--------------------------------------------------|--------------------|
|                                            Serial| 50000000.0000000000|
|                                  Naive std::async| 50000000.0000000000|
|                                parallel::reduce()| 50000000.0000000000|
|   parallel::reduce<4>() (loop unrolling enabled))| 50000000.0000000000|


====== BENCHMARKING ON: Repeated matrix multiplication ======

Threads           -> 4
N                 -> 600
repeats           -> 20
Data memory usage -> 8.23974609375 MiB

| relative |               ms/op |                op/s |    err% |     total | Repeated matrix multiplication
|---------:|--------------------:|--------------------:|--------:|----------:|:-------------------------------
|   100.0% |            1,112.58 |                0.90 |    0.0% |     80.11 | `Serial version`
|   292.3% |              380.69 |                2.63 |    0.1% |     27.26 | `OpenMP parallel for`
|   207.7% |              535.73 |                1.87 |   11.0% |     37.04 | `Naive std::async()`
|   427.3% |              260.39 |                3.84 |    0.3% |     18.24 | `parallel::task()`
|   429.3% |              259.14 |                3.86 |    0.1% |     18.68 | `parallel::for_loop()`

|----------------------------------------|--------------------|
|                                  Method|         Control sum|
|----------------------------------------|--------------------|
|                                  Serial|         1.07912e+09|
|                     OpenMP parallel for|         1.07912e+09|
|                      Naive std::async()|         1.07912e+09|
|                        parallel::task()|         1.07912e+09|
|                    parallel::for_loop()|         1.07912e+09|

// Note 1: Notice extremely unstable measurement for `std::async()` (aka large `err%`),
//         creating new threads is a highly unpredictable task due to OS scheduling.
//
// Note 2: Not sure why OpenMP doesn't give as much speedup as expected.
//
// Note 3: Speedup over 4x can happen on small matrices (like in this measurement)
//         due to utilization of multiple cache lines in a distributed case.
//         In case of reductions it is caused by SIMD unrolling, a version
//         with no unrolling performs similarly to OpenMP.
```
